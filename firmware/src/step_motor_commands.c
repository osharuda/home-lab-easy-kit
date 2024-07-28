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
 *   \brief Stepper Motor commands device C source file.
 *   \author Oleh Sharuda
 */

#include "fw.h"
#ifdef STEP_MOTOR_DEVICE_ENABLED

#include "utools.h"
#include "step_motor.h"
#include "step_motor_commands.h"

/// \addtogroup group_step_motor_dev_impl
/// @{

/// \brief This global array is used by #STEP_MOTOR_COMMAND_LENGTH macro to calculate stepper motor command length
const uint16_t g_step_motor_cmd_length_map[] = {STEP_MOTOR_8BIT_COMMAND_LEN, STEP_MOTOR_16BIT_COMMAND_LEN, STEP_MOTOR_24BIT_COMMAND_LEN, STEP_MOTOR_72BIT_COMMAND_LEN};

/// \brief This global array is served as map to optimize calling of stepper motor command handlers. Map initialization
///        is performed in step_motor_init_cmd_map()
PFN_STEP_MOTOR_CMD_FUNC g_step_motor_cmd_map[STEP_MOTOR_CMD_COUNT] = {step_motor_invalid_cmd};
/// @}

void step_motor_init_cmd_map(void) {
    // General commands
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_ENABLE] = step_motor_general_enable;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_SLEEP] = step_motor_general_sleep;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_DISABLE] = step_motor_general_disable;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_WAKEUP] = step_motor_general_wakeup;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_RESET] = step_motor_general_reset;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_WAIT] = step_motor_general_wait;
    g_step_motor_cmd_map[STEP_MOTOR_GENERAL | STEP_MOTOR_GENERAL_CONFIG] = step_motor_general_config;

    // Set commands
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_DIR_CW] = step_motor_set_dir_cw;
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_DIR_CCW] = step_motor_set_dir_ccw;
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_MICROSTEP] = step_motor_set_microstep;
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_STEP_WAIT] = step_motor_set_step_wait;
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_CW_SFT_LIMIT] = step_motor_set_cw_sft_limit;
    g_step_motor_cmd_map[STEP_MOTOR_SET | STEP_MOTOR_SET_CCW_SFT_LIMIT] = step_motor_set_ccw_sft_limit;

    // Move and move-non-stop commands
    for (uint8_t i=0; i<=STEP_MOTOR_ARG_MASK; i++) {
        g_step_motor_cmd_map[STEP_MOTOR_MOVE | i] = step_motor_move;
        g_step_motor_cmd_map[STEP_MOTOR_MOVE_NON_STOP | i] = step_motor_move;
    }
}

uint64_t step_motor_correct_timing(uint64_t wait, uint8_t corr_factor, struct StepMotorContext* mcontext) {
    uint64_t max_cor = wait >> corr_factor;
    if (max_cor==0) { // correction is not allowed
        return wait;
    }

    uint64_t min_wait = wait - max_cor;

    if (mcontext->late_us  > max_cor) {
        mcontext->late_us -= max_cor;
        return min_wait;
    } else {
        wait-=mcontext->late_us;
        mcontext->late_us = 0;
        return wait;
    }
}

void step_motor_handle_error(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    UNUSED(dev);
    UNUSED(mindex);
    UNUSED(cmd);
    assert_param(0);
}

uint8_t step_motor_invalid_cmd(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    UNUSED(cmd);
    UNUSED(dev);
    UNUSED(mindex);
    return STE_MOTOR_CMD_RESULT_FAIL;
}

uint8_t step_motor_general_enable(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_ENABLE_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_ENABLE, Bit_RESET);
    }

    DISABLE_IRQ
    CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_DISABLE_DEFAULT);
    ENABLE_IRQ

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_general_sleep(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_SLEEP_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_SLEEP, Bit_RESET);
    }

    DISABLE_IRQ
    CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_WAKEUP_DEFAULT);
    ENABLE_IRQ


    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_general_disable(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_ENABLE_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_ENABLE, Bit_SET);
    }

    DISABLE_IRQ
    SET_FLAGS(mstatus->motor_state, STEP_MOTOR_DISABLE_DEFAULT);
    ENABLE_IRQ

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_general_wakeup(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_SLEEP_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_SLEEP, Bit_SET);
    }

    DISABLE_IRQ
    SET_FLAGS(mstatus->motor_state, STEP_MOTOR_WAKEUP_DEFAULT);
    ENABLE_IRQ

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_general_reset(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    uint8_t res = STE_MOTOR_CMD_RESULT_OK;

    switch (cmd->state) {
        case STEP_MOTOR_CMDSTATUS_INIT:
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_RESET, Bit_RESET);
            step_motor_set_line(mdescr, STEP_MOTOR_LINE_RESET, Bit_SET);
            cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
            cmd->wait = 0;
            break;

        default:
            assert_param(0);
            res = STE_MOTOR_CMD_RESULT_FAIL;
    }

    return res;
}
uint8_t step_motor_general_wait(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    uint8_t res = STE_MOTOR_CMD_RESULT_OK;
    struct StepMotorContext* mcontext;
    UNUSED(dev);
    UNUSED(mindex);
    switch (cmd->state) {
        case STEP_MOTOR_CMDSTATUS_INIT:
            mcontext = MOTOR_CONTEXT(dev, mindex);
            cmd->state = STEP_MOTOR_CMDSTATUS_WAIT;
            cmd->wait = step_motor_correct_timing(cmd->param, STEP_MOTOR_CORRECTION_FACTOR, mcontext);
            break;
        case STEP_MOTOR_CMDSTATUS_WAIT:
            cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
            break;
        default:
            res = STE_MOTOR_CMD_RESULT_FAIL;
    }
    return res;
}

uint8_t step_motor_general_config(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    uint32_t value = STEP_MOTOR_CONFIG_BYTE_TO_FLAGS(cmd->param);
    DISABLE_IRQ
    SET_BIT_FIELD(mstatus->motor_state, STEP_MOTOR_CONFIG_MASK, value);
    ENABLE_IRQ
    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}

uint8_t step_motor_set_dir_cw(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_DIR_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_DIR, Bit_SET);
    }

    DISABLE_IRQ
    SET_FLAGS(mstatus->motor_state, STEP_MOTOR_DIRECTION_CW);
    ENABLE_IRQ

    step_motor_update_pos_change_by_step(mdescr, mstatus, mcontext);

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_set_dir_ccw(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);

    if (mdescr->config_flags & STEP_MOTOR_DIR_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_DIR, Bit_RESET);
    }

    DISABLE_IRQ
    CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_DIRECTION_CW);
    ENABLE_IRQ

    step_motor_update_pos_change_by_step(mdescr, mstatus, mcontext);

    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return STE_MOTOR_CMD_RESULT_OK;
}
uint8_t step_motor_set_microstep(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);
    uint8_t result = STE_MOTOR_CMD_RESULT_OK;
    BitAction m1,m2,m3;

    m1 = (cmd->param & STEP_MOTOR_SET_MICROSTEP_M1) == 0 ? Bit_RESET : Bit_SET;
    m2 = (cmd->param & STEP_MOTOR_SET_MICROSTEP_M2) == 0 ? Bit_RESET : Bit_SET;
    m3 = (cmd->param & STEP_MOTOR_SET_MICROSTEP_M3) == 0 ? Bit_RESET : Bit_SET;

    uint32_t flag = (m1 << STEP_MOTOR_M1_DEFAULT_OFFSET) | (m2 << STEP_MOTOR_M2_DEFAULT_OFFSET) | (m3 << STEP_MOTOR_M3_DEFAULT_OFFSET);
    DISABLE_IRQ
    SET_BIT_FIELD(mstatus->motor_state, STEP_MOTOR_M1_DEFAULT | STEP_MOTOR_M2_DEFAULT | STEP_MOTOR_M3_DEFAULT, flag);
    ENABLE_IRQ

    if (step_motor_update_pos_change_by_step(mdescr, mstatus, mcontext)) {
        result = STE_MOTOR_CMD_RESULT_FAIL;
        goto done;
    }

    if (mdescr->config_flags & STEP_MOTOR_M1_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M1, m1);
    }

    if (mdescr->config_flags & STEP_MOTOR_M2_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M2, m2);
    }

    if (mdescr->config_flags & STEP_MOTOR_M3_IN_USE) {
        step_motor_set_line(mdescr, STEP_MOTOR_LINE_M3, m3);
    }

done:
    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return result;
}
uint8_t step_motor_set_step_wait(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorContext* mcontext = MOTOR_CONTEXT(dev, mindex);
    uint8_t result = STE_MOTOR_CMD_RESULT_OK;
    if (cmd->param<STEP_MOTOR_MIN_STEP_WAIT) {
        result = STE_MOTOR_CMD_RESULT_FAIL;
    } else {
        mcontext->step_wait = cmd->param;
    }

    cmd->wait = 0;
    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return result;
}

uint8_t step_motor_set_cw_sft_limit(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    uint8_t result = STE_MOTOR_CMD_RESULT_OK;
    int64_t limit = (int64_t)cmd->param;

    if (limit<=mstatus->ccw_sft_limit) {
        // cw software limit may not be less or equal than ccw limit
        result = STE_MOTOR_CMD_RESULT_FAIL;
    } else {
        DISABLE_IRQ
        mstatus->cw_sft_limit = (int64_t)cmd->param;
        ENABLE_IRQ
    }



    cmd->wait = 0;
    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return result;
}

uint8_t step_motor_set_ccw_sft_limit(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    uint8_t result = STE_MOTOR_CMD_RESULT_OK;
    int64_t limit = (int64_t)cmd->param;

    if (limit>=mstatus->cw_sft_limit) {
        // ccw software limit may not greater or equal than cw limit
        result = STE_MOTOR_CMD_RESULT_FAIL;
    } else {
        DISABLE_IRQ
        mstatus->ccw_sft_limit = (int64_t)cmd->param;
        ENABLE_IRQ
    }

    cmd->wait = 0;
    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;
    return result;
}

uint8_t step_motor_move(struct StepMotorDevice* dev, uint8_t mindex, struct StepMotorCmd* cmd) {
    struct StepMotorDescriptor* mdescr = MOTOR_DESCR(dev, mindex);
    struct StepMotorContext*  mcontext = MOTOR_CONTEXT(dev, mindex);
    struct StepMotorStatus* mstatus = MOTOR_STATUS(dev, mindex);
    uint8_t direction = STEP_MOTOR_DIRECTION(mstatus->motor_state);
    uint8_t res = STE_MOTOR_CMD_RESULT_OK;

    do {
        switch (cmd->state) {
            case STEP_MOTOR_CMDSTATUS_INIT: {
                // Clear all endstops
                DISABLE_IRQ
                CLEAR_FLAGS(mstatus->motor_state, STEP_MOTOR_CW_ENDSTOP_TRIGGERED | STEP_MOTOR_CCW_ENDSTOP_TRIGGERED);
                ENABLE_IRQ

                // Mask/unmask endstops (this call must also set active endstop if required)
                uint8_t stop = step_motor_prepare_for_move(dev->dev_ctx.dev_index, mindex, cmd);
                cmd->state = (stop!=0) ? STEP_MOTOR_CMDSTATUS_DONE : STEP_MOTOR_CMDSTATUS_STEP;
            }
            break;

            case STEP_MOTOR_CMDSTATUS_STEP: {
                step_motor_set_line(mdescr, STEP_MOTOR_LINE_STEP, Bit_SET);
                step_motor_set_line(mdescr, STEP_MOTOR_LINE_STEP, Bit_RESET);

                cmd->wait = step_motor_correct_timing(mcontext->step_wait, STEP_MOTOR_CORRECTION_FACTOR, mcontext);

                DISABLE_IRQ
                mstatus->pos += mcontext->pos_change_by_step;
                ENABLE_IRQ

                cmd->state = STEP_MOTOR_CMDSTATUS_STEPWAIT;
            }
                break;

            case STEP_MOTOR_CMDSTATUS_STEPWAIT:
                cmd->param-=mcontext->step_counter_decrement;
                if (cmd->param==0) {
                    cmd->state = STEP_MOTOR_CMDSTATUS_DONE;

                    if (mcontext->move_sw_endstop_flag) {
                        // This move must be finished by software limit
                        assert_param(mstatus->pos<=mstatus->ccw_sft_limit || mstatus->pos>=mstatus->cw_sft_limit);
                        DISABLE_IRQ
                        SET_FLAGS(mstatus->motor_state, mcontext->move_sw_endstop_flag);
                        ENABLE_IRQ
                        uint8_t suspended = step_motor_handle_alarm(dev, mstatus, STEP_MOTOR_IGNORE_ENDSTOP_FLAG(direction), STEP_MOTOR_ALL_ENDSTOP_FLAG(direction));
                        if (suspended==0) {
                            // endstop was ignored, we have to continue move and restore command
                            if (STEP_MOTOR_LIMITED_MOVE(cmd->cmd)) {
                                cmd->param=mcontext->steps_beyond_endstop;
                            } else {
                                mcontext->step_counter_decrement = 0;
                                cmd->param = UINT64_MAX;
                            }
                            cmd->state = STEP_MOTOR_CMDSTATUS_STEP;
                        }
                    }
                } else {
                    cmd->state = STEP_MOTOR_CMDSTATUS_STEP;
                }
                break;

            default:
                res = STE_MOTOR_CMD_RESULT_FAIL;
                break;
        }
    } while (cmd->state==STEP_MOTOR_CMDSTATUS_STEP);

    return res;
}

#endif
