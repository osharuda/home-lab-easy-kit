/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief Stepper Motor device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"

#ifdef STEP_MOTOR_DEVICE_ENABLED

#include "timers.h"
#include "circbuffer.h"
#include <string.h>
#include "step_motor.h"
#include "step_motor_commands.h"
#include "sys_tick_counter.h"
#include "extihub.h"
#include "step_motor_conf.h"


STEP_MOTORS_BUFFERS
STEP_MOTOR_MOTOR_DESCRIPTORS
STEP_MOTOR_MOTOR_DESCRIPTOR_ARRAYS
STEP_MOTOR_MOTOR_CONTEXT_ARRAYS
STEP_MOTOR_MOTOR_STATUS_ARRAYS
STEP_MOTOR_DEV_STATUS_BUFFER
STEP_MOTOR_DEVICE_DESCRIPTORS

/// \addtogroup group_step_motor_dev_impl
/// @{

/// \brief This global variable keeps table of available microstep options for supported stepper motor drivers. See
/// #STEP_MOTOR_MICROSTEP_TABLE for details.
const StepMotorMicrostepTables g_step_motor_microstep_tables= STEP_MOTOR_MICROSTEP_TABLE;

/// \brief This global variable defines available stepper motor devices
struct StepMotorDevice* g_step_motor_devs[] = STEP_MOTOR_DEVICES;
/// @}

extern PFN_STEP_MOTOR_CMD_FUNC g_step_motor_cmd_map[STEP_MOTOR_CMD_COUNT];


static inline uint16_t step_motor_fetch_cmd(struct CircBuffer* circ, struct StepMotorCmd* cmd);
static inline uint8_t step_motor_get_ustep_bitshift(struct StepMotorDescriptor* mdescr, struct StepMotorStatus* mstatus, uint8_t* bitshift);

void STEP_MOTOR_COMMON_TIMER_IRQ_HANDLER(uint16_t dev_index) {
    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    if (TIMER_IS_UPDATE_EV(&dev->timer_data) == 0) {
        return;
    }

    uint64_t now = get_us_clock();
    step_motor_timer_event(dev, now);
    //TIM_ClearITPendingBit(dev->timer, TIM_IT_Update);
    TIMER_CLEAR_IT_PENDING_EV(&dev->timer_data);
}

STEP_MOTOR_FW_TIMER_IRQ_HANDLERS

void step_motors_suspend_all(struct StepMotorDevice* dev) {
    struct StepMotorStatus* mst = MOTOR_STATUS(dev,0);
    struct StepMotorStatus* mst_end = MOTOR_STATUS(dev, dev->motor_count);

    RECURSIVE_CRITICAL_SECTION_ENTER
    for (; mst!=mst_end; mst++) {
        SET_FLAGS(mst->motor_state, STEP_MOTOR_DONE | STEP_MOTOR_SUSPENDING);
    }
    RECURSIVE_CRITICAL_SECTION_LEAVE
}

uint8_t step_motor_handle_alarm(struct StepMotorDevice* dev,
                                struct StepMotorStatus* mstatus,
                             uint32_t ignore_flag,
                             uint32_t all_flag) {
    uint8_t res = 0;

    if (mstatus->motor_state & all_flag) {
        step_motors_suspend_all(dev);
        res = 1;
    } else if ((mstatus->motor_state & ignore_flag)==0) {
        // Stop just this one
        RECURSIVE_CRITICAL_SECTION_ENTER
        SET_FLAGS(mstatus->motor_state, STEP_MOTOR_DONE | STEP_MOTOR_SUSPENDING);
        RECURSIVE_CRITICAL_SECTION_LEAVE
        res = 1;
    }

    return res;
}

void step_motor_fault_handler(uint64_t clock, volatile void* ctx) {
    UNUSED(clock);
    uint8_t dev_index = STEP_MOTOR_EXTI_DEV_INDEX(ctx);
    uint8_t mindex = STEP_MOTOR_EXTI_MINDEX(ctx);

    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorLine* int_line = mdescr->lines + STEP_MOTOR_LINE_FAULT;
    uint8_t val = GPIO_ReadInputDataBit(int_line->port, 1<<int_line->pin);
    uint32_t inactive = (val << STEP_MOTOR_FAULT_ACTIVE_HIGH_OFFSET) ^ (mdescr->config_flags & STEP_MOTOR_FAULT_ACTIVE_HIGH);

    if (inactive==0) {
        RECURSIVE_CRITICAL_SECTION_ENTER
        SET_FLAGS(mstatus->motor_state, STEP_MOTOR_FAILURE);
        RECURSIVE_CRITICAL_SECTION_LEAVE

        step_motor_handle_alarm(dev, mstatus, STEP_MOTOR_CONFIG_FAILURE_IGNORE, STEP_MOTOR_CONFIG_FAILURE_ALL);
    }
}

void step_motor_cw_end_stop_handler(uint64_t clock, volatile void* ctx) {
    UNUSED(clock);
    uint8_t dev_index = STEP_MOTOR_EXTI_DEV_INDEX(ctx);
    uint8_t mindex = STEP_MOTOR_EXTI_MINDEX(ctx);

    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);

    struct StepMotorLine* int_line = mdescr->lines + STEP_MOTOR_LINE_CWENDSTOP;
    uint8_t val = GPIO_ReadInputDataBit(int_line->port, 1<<int_line->pin);
    uint32_t inactive = (val << STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH_OFFSET) ^ (mdescr->config_flags & STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH);
    uint8_t direction = STEP_MOTOR_DIRECTION(mstatus->motor_state);

    if (direction==STEP_MOTOR_SET_DIR_CW && inactive==0) {
        // we are in the same direction, it is true endstop interrupt
        RECURSIVE_CRITICAL_SECTION_ENTER
        SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CW_ENDSTOP_TRIGGERED);
        RECURSIVE_CRITICAL_SECTION_LEAVE

        // Mask EXTI pin to avoid repeated interrupt generation. It could be that endstop stuck in semi on state
        MASK_EXTI_PIN(int_line->pin);

        step_motor_handle_alarm(dev, mstatus, STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE, STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL);
    } else if (direction==STEP_MOTOR_SET_DIR_CCW && inactive!=0) {
        // endstop released
        RECURSIVE_CRITICAL_SECTION_ENTER
        CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_CW_ENDSTOP_TRIGGERED);
        RECURSIVE_CRITICAL_SECTION_LEAVE
    }
}

void step_motor_ccw_end_stop_handler(uint64_t clock, volatile void* ctx) {
    UNUSED(clock);
    uint8_t dev_index = STEP_MOTOR_EXTI_DEV_INDEX(ctx);
    uint8_t mindex = STEP_MOTOR_EXTI_MINDEX(ctx);

    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorLine* int_line = mdescr->lines + STEP_MOTOR_LINE_CCWENDSTOP;
    uint8_t val = GPIO_ReadInputDataBit(int_line->port, 1<<int_line->pin);
    uint32_t inactive = (val << STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH_OFFSET) ^ (mdescr->config_flags & STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH);
    uint8_t direction = STEP_MOTOR_DIRECTION(mstatus->motor_state);

    if (direction==STEP_MOTOR_SET_DIR_CCW && inactive==0) {
        // we are in the same direction, it is true endstop interrupt
        RECURSIVE_CRITICAL_SECTION_ENTER
        SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CCW_ENDSTOP_TRIGGERED);
        RECURSIVE_CRITICAL_SECTION_LEAVE

        // Mask EXTI pin to avoid repeated interrupt generation. It could be that endstop stuck in semi on state
        MASK_EXTI_PIN(int_line->pin);

        step_motor_handle_alarm(dev, mstatus, STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE, STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL);
    } else if (direction==STEP_MOTOR_SET_DIR_CW && inactive!=0) {
        // endstop released
        RECURSIVE_CRITICAL_SECTION_ENTER
        CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_CCW_ENDSTOP_TRIGGERED);
        RECURSIVE_CRITICAL_SECTION_LEAVE
    }
}

void step_motor_init_motor_line(struct StepMotorDescriptor* mdescr, uint8_t linenum) {
    START_PIN_DECLARATION
    assert_param(linenum < sizeof(mdescr->lines) / sizeof(struct StepMotorLine));
    assert_param((linenum != STEP_MOTOR_LINE_FAULT) &&
                      (linenum != STEP_MOTOR_LINE_CWENDSTOP) &&
                      (linenum != STEP_MOTOR_LINE_CCWENDSTOP)); // these lines can't be set
    assert_param(mdescr->lines[linenum].port!=0);

    uint16_t pin_mask = 1 << mdescr->lines[linenum].pin;
    DECLARE_PIN(mdescr->lines[linenum].port, pin_mask, GPIO_Mode_Out_PP)
}

void step_motor_set_line(struct StepMotorDescriptor* mdescr, uint8_t linenum, BitAction value) {
    assert_param(linenum < sizeof(mdescr->lines) / sizeof(struct StepMotorLine));
    assert_param((linenum != STEP_MOTOR_LINE_FAULT) &&
                 (linenum != STEP_MOTOR_LINE_CWENDSTOP) &&
                 (linenum != STEP_MOTOR_LINE_CCWENDSTOP)); // these lines can't be set
    assert_param(mdescr->lines[linenum].port!=0);
    struct StepMotorLine* line = (struct StepMotorLine*) mdescr->lines + linenum;
    GPIO_WriteBit(line->port, 1 << line->pin, value);
}

uint8_t step_motor_init_exti(struct StepMotorDescriptor* mdescr, uint8_t linenum, uint16_t exti_cr, uint16_t active_high,
                          PFN_EXTIHUB_CALLBACK callback, uint8_t dev_index, uint8_t mindex) {
    assert_param(linenum < sizeof(mdescr->lines) / sizeof(struct StepMotorLine));
    assert_param((linenum == STEP_MOTOR_LINE_FAULT) ||
                      (linenum == STEP_MOTOR_LINE_CWENDSTOP) ||
                      (linenum == STEP_MOTOR_LINE_CCWENDSTOP));
    assert_param(mdescr->lines[linenum].port!=0);

    GPIOMode_TypeDef mode = active_high!=0 ? GPIO_Mode_IPD : GPIO_Mode_IPU;

    // Note, by default endstops are disabled to generate interrupts
    return exti_register_callback(mdescr->lines[linenum].port,
                                  mdescr->lines[linenum].pin,
                                  mode,
                                  exti_cr,
                                  1, 1,
                                  callback,
                                  STEP_MOTOR_EXTI_PARAM(dev_index,mindex),
                                  1);
}

uint8_t step_motor_mask_exti(struct StepMotorDescriptor* mdescr, uint8_t linenum) {
    assert_param(linenum < sizeof(mdescr->lines) / sizeof(struct StepMotorLine));
    assert_param((linenum == STEP_MOTOR_LINE_FAULT) || (linenum == STEP_MOTOR_LINE_CWENDSTOP) || (linenum == STEP_MOTOR_LINE_CCWENDSTOP));
    assert_param(mdescr->lines[linenum].port!=0);

    return exti_mask_callback(mdescr->lines[linenum].port, mdescr->lines[linenum].pin);
}

static inline uint8_t step_motor_get_ustep_bitshift(struct StepMotorDescriptor* mdescr, struct StepMotorStatus* mstatus, uint8_t* bitshift) {
    uint8_t mval = STEP_MOTOR_MICROSTEP_STATUS_TO_VALUE(mstatus->motor_state);
    *bitshift = g_step_motor_microstep_tables[mdescr->motor_driver][mval];
    uint8_t res = 0;

    if (*bitshift == STEP_MOTOR_BAD_STEP) {
        assert_param(0);              // we shouldn't be here
        *bitshift = STEP_MOTOR_FULL_STEP;  // Treat as full step in release configurations
        res = 1;                           // indicate error for release
    }

    return res;
}

uint8_t step_motor_prepare_for_move(uint8_t dev_index, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorContext*  mcontext = MOTOR_CONTEXT(dev, mindex);

    uint8_t limited_move = STEP_MOTOR_LIMITED_MOVE(cmd->cmd);
    uint8_t stop_command_exec = 0; // do not stop command

    static const PFN_EXTIHUB_CALLBACK endstop_handlers[] = {step_motor_ccw_end_stop_handler, step_motor_cw_end_stop_handler};
    uint8_t direction = STEP_MOTOR_DIRECTION(mstatus->motor_state);
    assert_param(direction==0 || direction==1);

    uint32_t active_endstop = STEP_MOTOR_DIRECTION_TO_ACTIVE_ENDSTOP(direction);
    uint32_t inactive_endstop = STEP_MOTOR_DIRECTION_TO_INACTIVE_ENDSTOP(direction);
    uint8_t active_endstop_line = STEP_MOTOR_ENDSTOP_TO_LINE(active_endstop);
    uint8_t inactive_endstop_line = STEP_MOTOR_ENDSTOP_TO_LINE(inactive_endstop);
    uint32_t active_endstop_used = STEP_MOTOR_IS_USED_ENDSTOP(mdescr->config_flags, active_endstop);
    uint32_t inactive_endstop_used = STEP_MOTOR_IS_USED_ENDSTOP(mdescr->config_flags, inactive_endstop);
    uint32_t ignore_flag, for_all_flag;

    // Clear software endstop flag
    mcontext->move_sw_endstop_flag = 0;
    mcontext->steps_beyond_endstop = 0;

    // Mask inactive endstop
    if (inactive_endstop_used) {
        MASK_EXTI_PIN(mdescr->lines[inactive_endstop_line].pin);
    }

    if (limited_move) {
        // always use decrement for limited movements with parameters
        mcontext->step_counter_decrement = 1;
        stop_command_exec = (cmd->param==0) ? 1 : 0;
    } else {
        // Set param very high to indicate unlimited moves
        cmd->param = UINT64_MAX;
        mcontext->step_counter_decrement = 0;
    }

    // Unmask active endstop or initialize software endstop
    if (active_endstop_used) {
        // Preset current values by calling EXTI handler from here
        endstop_handlers[direction](0, STEP_MOTOR_EXTI_PARAM(dev_index, mindex));

        // If active endstop didn't trigger during call above - unmask it
        if ((mstatus->motor_state & active_endstop)==0) {
            UNMASK_EXTI_PIN(mdescr->lines[active_endstop_line].pin);
        }
    } else {
        // For software endstop always use decrement
        mcontext->step_counter_decrement = 1;

        // Calculate number of steps to reach software endstop
        uint8_t bitshift;
        step_motor_get_ustep_bitshift(mdescr, mstatus, &bitshift);

        int64_t diff;
        if (direction) {
            diff = mstatus->cw_sft_limit - mstatus->pos;
            ignore_flag = STEP_MOTOR_CONFIG_CW_ENDSTOP_IGNORE;
            for_all_flag = STEP_MOTOR_CONFIG_CW_ENDSTOP_ALL;
        } else {
            diff = mstatus->pos - mstatus->ccw_sft_limit;
            ignore_flag = STEP_MOTOR_CONFIG_CCW_ENDSTOP_IGNORE;
            for_all_flag = STEP_MOTOR_CONFIG_CCW_ENDSTOP_ALL;
        }

        assert_param(mcontext->pos_change_by_step!=0);

        // Number of steps (pulses) we have to do to reach (possibly to go a bit beyond because of micro stepping) the endstop.
        // Normally this value should be positive. Negative values or zero indicate we are beyond software limit.
        uint64_t nstep = diff >> bitshift;
        if ((nstep << bitshift)!=(uint64_t)diff) {
            nstep++; // not even number of micro steps, add a bit to make sure we hit software limit reliably.
        }

        // Calculate endstop flag at the moment when move finishes
        if ( (diff > 0) && (cmd->param>=nstep)) {
            mcontext->move_sw_endstop_flag = active_endstop;
            mcontext->steps_beyond_endstop = cmd->param - nstep;
            cmd->param = nstep;
        } else if (diff <= 0) {
            // it seems like we already have passed software limits
            assert_param(mstatus->pos<=mstatus->ccw_sft_limit || mstatus->pos>=mstatus->cw_sft_limit);
            RECURSIVE_CRITICAL_SECTION_ENTER
            SET_FLAGS(mstatus->motor_state, active_endstop);
            RECURSIVE_CRITICAL_SECTION_LEAVE
            stop_command_exec = step_motor_handle_alarm(dev, mstatus, ignore_flag, for_all_flag);
        }
    }

    return stop_command_exec;
}

void step_motor_suspend_motor(struct StepMotorDevice* dev,
                              struct StepMotorDescriptor* mdescr,
                              struct StepMotorStatus* mstatus,
                              uint8_t error) {
    uint32_t mcfg = mdescr->config_flags;

    uint32_t mask = STEP_MOTOR_DONE | STEP_MOTOR_SUSPENDING;
    uint32_t flags = STEP_MOTOR_DONE;
    if (error) {
        mask |= STEP_MOTOR_ERROR;
        flags |= STEP_MOTOR_ERROR;

        if (mstatus->motor_state & STEP_MOTOR_CONFIG_ERROR_ALL) {
            // stop all motors
            step_motors_suspend_all(dev);
        }
    }

    RECURSIVE_CRITICAL_SECTION_ENTER
    SET_BIT_FIELD(mstatus->motor_state, mask, flags);
    RECURSIVE_CRITICAL_SECTION_LEAVE

    // initialize GPIO : ENABLE (optional), default state: LOW
    if (mcfg & STEP_MOTOR_ENABLE_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_ENABLE, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_DISABLE_DEFAULT_OFFSET));
    }

    // initialize GPIO : SLEEP (optional), default state: HIGH
    if (mcfg & STEP_MOTOR_SLEEP_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_SLEEP, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_WAKEUP_DEFAULT_OFFSET));
    }
}

void step_motor_resume_motor(struct StepMotorDescriptor* mdescr, struct StepMotorStatus* mstatus) {
    uint32_t mcfg = mdescr->config_flags;
    // initialize GPIO : ENABLE (optional) with preserved power state
    if (mcfg & STEP_MOTOR_ENABLE_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_ENABLE, TO_ZERO_OR_ONE(mstatus->motor_state, STEP_MOTOR_DISABLE_DEFAULT_OFFSET));
    }

    // initialize GPIO : SLEEP (optional) with preserved power state
    if (mcfg & STEP_MOTOR_SLEEP_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_SLEEP, TO_ZERO_OR_ONE(mstatus->motor_state, STEP_MOTOR_WAKEUP_DEFAULT_OFFSET));
    }
}

void step_motor_init_gpio_and_exti(struct StepMotorDevice* dev) {
    for (uint8_t mindex=0; mindex<dev->motor_count; mindex++) {
        struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
        struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

        // Reset motor state to default
        mstatus->motor_state = mdescr->config_flags;
        uint32_t mcfg = mstatus->motor_state;
        uint8_t pin_val;

        if (mcfg & STEP_MOTOR_ENABLE_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_ENABLE);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_ENABLE, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_DISABLE_DEFAULT_OFFSET));
        }

        // initialize GPIO : SLEEP (optional) with preserved power state
        if (mcfg & STEP_MOTOR_SLEEP_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_SLEEP);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_SLEEP, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_WAKEUP_DEFAULT_OFFSET));
        }

        // initialize GPIO : STEP, default state: LOW
        step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_STEP);
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_STEP, Bit_RESET);

        // initialize GPIO : DIR (optional)
        if (mcfg & STEP_MOTOR_DIR_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_DIR);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_DIR, STEP_MOTOR_DIRECTION(mcfg));
        }

        // initialize GPIO : M1 (optional)
        if (mcfg & STEP_MOTOR_M1_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_M1);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_M1, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M1_DEFAULT_OFFSET));
        }

        // initialize GPIO : M2 (optional)
        if (mcfg & STEP_MOTOR_M2_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_M2);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_M2, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M2_DEFAULT_OFFSET));
        }

        // initialize GPIO : M3 (optional)
        if (mcfg & STEP_MOTOR_M3_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_M3);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_M3, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M3_DEFAULT_OFFSET));
        }

        // initialize GPIO : RESET (optional), default state: HIGH
        if (mcfg & STEP_MOTOR_RESET_IN_USE) {
            step_motor_init_motor_line(mdescr, STEP_MOTOR_LINE_RESET);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_RESET, Bit_SET);
        }

        // initialize FAULT
        if (mcfg & STEP_MOTOR_FAULT_IN_USE) {
            pin_val = step_motor_init_exti(mdescr, STEP_MOTOR_LINE_FAULT, mdescr->fault_exticr, mcfg & STEP_MOTOR_FAULT_ACTIVE_HIGH,
                                           step_motor_fault_handler, dev->dev_ctx.dev_index, mindex);

            if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_FAULT_ACTIVE_HIGH)!=0) ) {
                RECURSIVE_CRITICAL_SECTION_ENTER
                SET_FLAGS(mstatus->motor_state, STEP_MOTOR_FAILURE);
                RECURSIVE_CRITICAL_SECTION_LEAVE
            }
        }

        // initialize CW ENDSTOP
        if (mcfg & STEP_MOTOR_CWENDSTOP_IN_USE) {
            pin_val = step_motor_init_exti(mdescr, STEP_MOTOR_LINE_CWENDSTOP, mdescr->cw_endstop_exticr, mcfg & STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH,
                                           step_motor_cw_end_stop_handler, dev->dev_ctx.dev_index, mindex);

            if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH)!=0) ) {
                RECURSIVE_CRITICAL_SECTION_ENTER
                SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CW_ENDSTOP_TRIGGERED);
                RECURSIVE_CRITICAL_SECTION_LEAVE
            }
        }

        // initialize CCW ENDSTOP
        if (mcfg & STEP_MOTOR_CCWENDSTOP_IN_USE) {
            pin_val = step_motor_init_exti(mdescr, STEP_MOTOR_LINE_CCWENDSTOP, mdescr->ccw_endstop_exticr, mcfg & STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH,
                                           step_motor_ccw_end_stop_handler, dev->dev_ctx.dev_index, mindex);

            if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH)!=0) ) {
                RECURSIVE_CRITICAL_SECTION_ENTER
                SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CCW_ENDSTOP_TRIGGERED);
                RECURSIVE_CRITICAL_SECTION_LEAVE
            }
        }
    } // end of for
}


void step_motor_set_default(struct StepMotorDevice* dev, uint8_t mindex) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    uint32_t mcfg = mstatus->motor_state;
    uint8_t pin_val;

    // initialize GPIO : STEP, default state: LOW
    step_motor_set_line(mdescr, STEP_MOTOR_LINE_STEP, Bit_RESET);

    // initialize GPIO : DIR (optional)
    if (mcfg & STEP_MOTOR_DIR_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_DIR, STEP_MOTOR_DIRECTION(mcfg));
    }

    // initialize GPIO : M1 (optional)
    if (mcfg & STEP_MOTOR_M1_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M1, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M1_DEFAULT_OFFSET));
    }

    // initialize GPIO : M2 (optional)
    if (mcfg & STEP_MOTOR_M2_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M2, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M2_DEFAULT_OFFSET));
    }

    // initialize GPIO : M3 (optional)
    if (mcfg & STEP_MOTOR_M3_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M3, TO_ZERO_OR_ONE(mcfg, STEP_MOTOR_M3_DEFAULT_OFFSET));
    }

    // initialize GPIO : RESET (optional), default state: HIGH
    if (mcfg & STEP_MOTOR_RESET_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_RESET, Bit_SET);
    }

    // initialize FAULT
    if (mcfg & STEP_MOTOR_FAULT_IN_USE) {
        // Mute interrupt
        pin_val = step_motor_mask_exti(mdescr, STEP_MOTOR_LINE_FAULT);

        if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_FAULT_ACTIVE_HIGH)!=0) ) {
            RECURSIVE_CRITICAL_SECTION_ENTER
            SET_FLAGS(mstatus->motor_state, STEP_MOTOR_FAILURE);
            RECURSIVE_CRITICAL_SECTION_LEAVE
        }
    }

    // initialize CW ENDSTOP
    if (mcfg & STEP_MOTOR_CWENDSTOP_IN_USE) {
        pin_val = step_motor_mask_exti(mdescr, STEP_MOTOR_LINE_CWENDSTOP);

        if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_CWENDSTOP_ACTIVE_HIGH)!=0) ) {
            RECURSIVE_CRITICAL_SECTION_ENTER
            SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CW_ENDSTOP_TRIGGERED);
            RECURSIVE_CRITICAL_SECTION_LEAVE
        }
    }

    // initialize CCW ENDSTOP
    if (mcfg & STEP_MOTOR_CCWENDSTOP_IN_USE) {
        pin_val = step_motor_mask_exti(mdescr, STEP_MOTOR_LINE_CCWENDSTOP);

        if ( (pin_val!=0) == ((mcfg & STEP_MOTOR_CCWENDSTOP_ACTIVE_HIGH)!=0) ) {
            RECURSIVE_CRITICAL_SECTION_ENTER
            SET_FLAGS(mstatus->motor_state, STEP_MOTOR_CCW_ENDSTOP_TRIGGERED);
            RECURSIVE_CRITICAL_SECTION_LEAVE
        }
    }
}

//---------------------------- DEVICE FUNCTIONS ----------------------------

void step_motor_set_dev_status(struct StepMotorDevice* dev, uint8_t mask, uint8_t flags) {
    assert_param((flags & mask)==flags);
    struct StepMotorDevStatus* dev_status = MOTOR_DEV_STATUS(dev);
    RECURSIVE_CRITICAL_SECTION_ENTER
    dev_status->status = (dev_status->status & (~mask)) | flags;
    RECURSIVE_CRITICAL_SECTION_LEAVE
}

void step_motor_init(void) {
    step_motor_init_cmd_map();
    for (uint8_t dev_index=0; dev_index<STEP_MOTOR_DEVICE_COUNT; dev_index++) {
        struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
        struct DeviceContext* dev_ctx = (struct DeviceContext*)&(dev->dev_ctx);

        // initialize device
        memset((void*)dev_ctx, 0, sizeof(struct DeviceContext));
        dev_ctx->device_id = dev->dev_id;
        dev_ctx->on_command = step_motor_dev_execute;
        dev_ctx->on_sync = step_motor_dev_sync;
        dev_ctx->buffer = (uint8_t*)(dev->status);
        dev_ctx->bytes_available = dev->status_size;
        dev_ctx->dev_index = dev_index;

        // Initialize timer preinit cache
        timer_init(&dev->timer_data, IRQ_PRIORITY_STEP_MOTOR_TIMER, TIM_CounterMode_Up, TIM_CKD_DIV1);

        // Test for alignment
        for (int i=0; i<dev->motor_count; i++) {
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].pos));
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].cw_sft_limit));
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].ccw_sft_limit));
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].motor_state));
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].bytes_remain));
            IS_SIZE_ALIGNED(&(dev->status->mstatus[i].reserved));
        }

        comm_register_device(dev_ctx);

        // initialize GPIO and EXTI
        step_motor_init_gpio_and_exti(dev);
        step_motor_dev_reset(dev, 1);
    }
}

static inline uint16_t step_motor_fetch_cmd(struct CircBuffer* circ, struct StepMotorCmd* cmd) {
    uint8_t c;
    uint16_t bytes_remain = 0;
    circbuf_start_read(circ);

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE; // mark command as no command

    // read command byte
    uint8_t res = circbuf_get_byte(circ, &c);
    uint16_t len = STEP_MOTOR_COMMAND_LENGTH(c);

    // param is put as little endian
    if (len>1) {
        assert_param(len <= (1+sizeof(cmd->param)));
        cmd->param = 0;
        uint8_t* pb = (uint8_t*)(&cmd->param);
        uint8_t* p_stop = pb + (len - 1);

        do {
            res = res & circbuf_get_byte(circ, pb);
            pb++;
        } while (pb<p_stop);

    } else {
        cmd->param = (uint64_t)(c & STEP_MOTOR_ARG_MASK);
    }

    // !!! optimization !!!
    // circbuf_get_byte() returns 1 for overflow, and 0 for success. All operations above may impact the following:
    // cmd->param ) In this case called doesn't take a look on param if cmd->state is STEP_MOTOR_CMDSTATUS_DONE
    // circ ) Circular buffer is not affected if circbuf_stop_read() is not called.
    // STEP_MOTOR_COMMAND_LENGTH ) macro can't go outside g_step_motor_cmd_length_map if any random value is passed.
    // All three above gives us green light to analyze res just here, we are ok for some penalty in the case of buffer overrun here.
    if (res!=0) {

        bytes_remain = circbuf_stop_read(circ, len);
        cmd->cmd = c;
        cmd->state = STEP_MOTOR_CMDSTATUS_INIT;
        cmd->wait = 0;
    }

    return bytes_remain;
}

// <TODO> Inline this function
void step_motor_timer_event(struct StepMotorDevice* dev, uint64_t now) {
    uint32_t w = MCU_MAXIMUM_TIMER_US;
    struct StepMotorDevPrivData* priv_data = MOTOR_DEV_PRIV_DATA(dev);
    uint64_t last_wait = now - priv_data->last_event_timestamp;
    uint8_t all_done = 1;
    uint8_t any_error = 0;

    for (uint8_t mindex=0; mindex<dev->motor_count; mindex++) {
        struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

        if (mstatus->motor_state & (STEP_MOTOR_DONE | STEP_MOTOR_ERROR)) {
            if (mstatus->motor_state & STEP_MOTOR_SUSPENDING) {
                step_motor_suspend_motor(dev, MOTOR_DESCR(dev, mindex), mstatus, 0);
            }

            any_error |= mstatus->motor_state &  STEP_MOTOR_ERROR;
            goto next_motor;   // error, don't do anything to this motor
        }

        struct StepMotorCmd* cmd = MOTOR_CMD(dev, mindex);

        if (last_wait >= cmd->wait) {
            struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);
            mcontext->late_us += last_wait - cmd->wait;

            uint8_t res = STE_MOTOR_CMD_RESULT_OK;

            do {
                // Check if new command should be read
                if (cmd->state==STEP_MOTOR_CMDSTATUS_DONE) {
                    uint16_t bytes_remain = step_motor_fetch_cmd((struct CircBuffer*)&(mcontext->circ_buffer), cmd);
                    RECURSIVE_CRITICAL_SECTION_ENTER
                    mstatus->bytes_remain = bytes_remain;
                    RECURSIVE_CRITICAL_SECTION_LEAVE

                    if (cmd->state==STEP_MOTOR_CMDSTATUS_DONE) {
                        // No commands read
                        cmd->wait = 0;
                        step_motor_suspend_motor(dev, MOTOR_DESCR(dev, mindex), mstatus, 0);
                        goto next_motor;
                    }
                }

                uint8_t cmd_index = cmd->cmd & (STEP_MOTOR_CMD_MASK | STEP_MOTOR_ARG_MASK);
                res = g_step_motor_cmd_map[cmd_index](dev, mindex, cmd);
                assert_param((res==STE_MOTOR_CMD_RESULT_OK) || (res==STE_MOTOR_CMD_RESULT_FAIL));

            } while (res==STE_MOTOR_CMD_RESULT_OK && cmd->state == STEP_MOTOR_CMDSTATUS_DONE);

            if (res==STE_MOTOR_CMD_RESULT_FAIL) {

                step_motor_suspend_motor(dev, MOTOR_DESCR(dev, mindex), mstatus, 1);
                goto next_motor;
            }
        } else {
            cmd->wait -= last_wait;
        }

        all_done = 0;

        if (cmd->wait < w) {
            w = cmd->wait;
        }

        next_motor:
        ;
    }

    if (all_done==0) {
        priv_data->last_event_timestamp = now;

        uint16_t prescaller, period;
        timer_get_params(w, &prescaller, &period);
        periodic_timer_start(&dev->timer_data, prescaller, period);

    } else {
        timer_disable(&dev->timer_data);
        step_motor_set_dev_status(dev, STEP_MOTOR_DEV_STATUS_STATE_MASK, any_error ? STEP_MOTOR_DEV_STATUS_ERROR : STEP_MOTOR_DEV_STATUS_IDLE);
    }
}

void step_motor_dev_start(struct StepMotorDevice* dev) {
    struct StepMotorDevPrivData* priv_data = MOTOR_DEV_PRIV_DATA(dev);
    step_motor_dev_reset(dev, 0);
    step_motor_set_dev_status(dev, STEP_MOTOR_DEV_STATUS_STATE_MASK, STEP_MOTOR_DEV_STATUS_RUN);

    priv_data->last_event_timestamp = get_us_clock();
    step_motor_timer_event(dev, priv_data->last_event_timestamp);
}

void step_motor_dev_stop(struct StepMotorDevice* dev) {
    step_motor_dev_reset(dev, 1); // stop everything, drop existing commands
}

uint8_t step_motor_update_pos_change_by_step(struct StepMotorDescriptor* mdescr, struct StepMotorStatus* mstatus, struct StepMotorContext* mcontext) {
    // Calculate default delta
    uint8_t bitshift;
    uint8_t res = step_motor_get_ustep_bitshift(mdescr, mstatus, &bitshift);
    uint8_t direction = STEP_MOTOR_DIRECTION(mstatus->motor_state);

    // Note: if CW direction==STEP_MOTOR_SET_DIR_CW==1, if CCW direction==STEP_MOTOR_SET_DIR_CCW==0
    // Thus the following optimization is valid: CW -> +1, CCW -> -1
    int8_t d = (int8_t) (direction << 1) - (int8_t)(1);
    mcontext->pos_change_by_step = d * STEP_MOTOR_MICROSTEP_DELTA(bitshift);

    return res;
}

void step_motor_dev_reset(struct StepMotorDevice* dev, uint8_t full_reset) {
    struct StepMotorDevStatus* dev_status = MOTOR_DEV_STATUS(dev);

    timer_disable(&dev->timer_data);

    RECURSIVE_CRITICAL_SECTION_ENTER
    dev_status->status = STEP_MOTOR_DEV_STATUS_IDLE;

    for (uint8_t mindex=0; mindex<dev->motor_count; mindex++) {
        struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);
        struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
        struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

        // Handle motor context
        if (full_reset) {
            circbuf_init((struct CircBuffer*) &mcontext->circ_buffer, mdescr->buffer, mdescr->buffer_size);

            mcontext->step_wait = mdescr->default_speed;

            // Clear motor status
            mstatus->motor_state = mdescr->config_flags;
            mstatus->bytes_remain = 0;
            mstatus->pos = 0;
            mstatus->cw_sft_limit = mdescr->cw_sft_limit;
            mstatus->ccw_sft_limit = mdescr->ccw_sft_limit;
            mstatus->reserved = 0;
        } else {
            CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_FAILURE | STEP_MOTOR_ERROR | STEP_MOTOR_DONE | STEP_MOTOR_SUSPENDING);
        }

        step_motor_update_pos_change_by_step(mdescr, mstatus, mcontext);
        mcontext->current_cmd.state = STEP_MOTOR_CMDSTATUS_DONE;
        mcontext->late_us = 0;

        // Put step motor into preserved/default power state
        step_motor_resume_motor(mdescr, mstatus);

        // Put motor GPIO into default state (this will also may affect motor status flags
        step_motor_set_default(dev, mindex);
    }
    RECURSIVE_CRITICAL_SECTION_LEAVE;
}


uint8_t step_motor_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {
    uint8_t mindex = 0;
    uint16_t len;
    uint16_t dev_index = comm_dev_context(cmd_byte)->dev_index;
    uint8_t protocol_status = COMM_STATUS_OK;
    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);
    struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);
    struct StepMotorStatus* mstatus;

    // read the buffer and copy commands to motor circular buffers
    struct CircBuffer* circ = (struct CircBuffer*)&mcontext->circ_buffer;

    for (uint16_t i=0; i<length; i+=len) {
        // Read command byte
        uint8_t cmd;
        uint16_t stop_index;

        cmd = data[i];
        if (cmd & STEP_MOTOR_SELECT) {
            mindex = cmd & (~STEP_MOTOR_SELECT);
            mcontext = MOTOR_CONTEXT(dev, mindex);
            circ = (struct CircBuffer*)&mcontext->circ_buffer;
            len = 1;
            continue;
        }

        len = STEP_MOTOR_COMMAND_LENGTH(cmd);

        stop_index = i+len;
        if (stop_index > length) {
            // command goes beyond buffer length
            step_motors_suspend_all(dev);
            step_motor_set_dev_status(dev, STEP_MOTOR_DEV_STATUS_ERROR, STEP_MOTOR_DEV_STATUS_ERROR);
            protocol_status |= COMM_STATUS_FAIL;
            goto done;
        }

        // copy data
        for (uint16_t j=i;j<stop_index;j++) {
            circbuf_put_byte(circ, data[j]);
        }
    }

    // Update bytes_remain values for each motor
    mcontext = MOTOR_CONTEXT(dev, 0);
    mstatus = MOTOR_STATUS(dev, 0);
    for (uint8_t i=0; i<dev->motor_count; i++, mstatus++, mcontext++) {
        circ = (struct CircBuffer*)&mcontext->circ_buffer;
        uint16_t bytes_remain = circbuf_len(circ);

        RECURSIVE_CRITICAL_SECTION_ENTER
        mstatus->bytes_remain = bytes_remain;
        RECURSIVE_CRITICAL_SECTION_LEAVE
    }


    // Do action if any command is specified
    switch (cmd_byte & (~COMM_MAX_DEV_ADDR)) {
        case STEP_MOTOR_START:
            step_motor_dev_start(dev);
        break;

        case STEP_MOTOR_STOP:
            step_motor_dev_stop(dev);
        break;
    }
done:
    return protocol_status;
}


uint8_t step_motor_dev_sync(uint8_t cmd_byte, uint16_t length) {
    UNUSED(length);
    uint16_t dev_index = comm_dev_context(cmd_byte)->dev_index;
    struct StepMotorDevice* dev = MOTOR_DEVICE(dev_index);

    // Copy data from internal buffer to external (used for read by i2c)
    RECURSIVE_CRITICAL_SECTION_ENTER
    /// It is safe to copy status information because device have COMM_STATUS_BUSY status at the moment. All status
    /// reads should fail because of this reason.
    memcpy(dev->status, dev->priv_data.internal_status, dev->status_size);
    RECURSIVE_CRITICAL_SECTION_LEAVE
    return COMM_STATUS_OK;
}

#endif

