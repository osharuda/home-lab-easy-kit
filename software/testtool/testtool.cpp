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
 *   \brief Testtool utility implementation
 *   \author Oleh Sharuda
 */

#include <iostream>
#include "tools.hpp"
#include "texttools.hpp"
#include <cassert>
#include "testtool.hpp"
#include <cctype>
#include <algorithm>
#include "utools.h"
#include "circbuffer.h"
#include "i2c_proto.h"
#include "math.h"

void test_stm32_timer_params_integer() {
    DECLARE_TEST(test_stm32_timer_params_integer)
    uint32_t us;
    uint16_t prescaller, period;

    REPORT_CASE
    {
        g_assert_param_count = 0;
        us = 0;
        prescaller = 0;
        period = 0;
        timer_get_params(us, &prescaller, &period);
        assert(g_assert_param_count==0);
        assert(prescaller == 0);
        assert(period == 0);
    }

    REPORT_CASE
    {
        g_assert_param_count = 0;
        us = 1;
        prescaller = 0;
        period = 0;
        timer_get_params(us, &prescaller, &period);
        assert(g_assert_param_count==0);
        assert(prescaller == 0);
        assert(period == MCU_FREQUENCY_MHZ-1);
    }    

    REPORT_CASE
    {
        g_assert_param_count = 0;
        us = MCU_MAXIMUM_TIMER_US;
        prescaller = 0;
        period = 0;
        timer_get_params(us, &prescaller, &period);
        assert(g_assert_param_count==0);
        assert(prescaller == 65535);
        assert(period >= 65534);
    }    

    REPORT_CASE
    {
        g_assert_param_count = 0;
        us = MCU_MAXIMUM_TIMER_US-1;
        prescaller = 0;
        period = 0;
        timer_get_params(us, &prescaller, &period);
        assert(g_assert_param_count==0);
        assert(prescaller == 65535);
        assert(period >= 65534);
    }

    // compare with stm32_timer_params
    REPORT_CASE
    {
        g_assert_param_count = 0;
        double expected = 0.0;
        uint32_t us;
        int res;
        uint32_t maxdiff1 = 0;
        uint32_t maxdiff2 = 0;
        uint64_t sd1 = 0;
        uint64_t sd2 = 0;

        int d1_g_d2 = 0;
        int d2_g_d1 = 0;
        uint16_t ps1, pd1, ps2, pd2;


        for (uint32_t us = 0; us<MCU_MAXIMUM_TIMER_US; us++) {
            uint16_t ps1, pd1, ps2, pd2;

            res = tools::stm32_timer_params(MCU_FREQUENCY, 1.0e-6L*(double)us, ps1, pd1, expected);
            assert(res==0);
            timer_get_params(us, &ps2, &pd2);

            uint32_t exp1 = ((uint64_t)pd1+1)*((uint64_t)ps1+1)/MCU_FREQUENCY_MHZ;
            uint32_t exp2 = ((uint64_t)pd2+1)*((uint64_t)ps2+1)/MCU_FREQUENCY_MHZ;

            uint32_t d1 = MDIFF(exp1, us);
            uint32_t d2 = MDIFF(exp2, us);

            assert(d1<1000000);
            assert(d2<1000000);

            uint32_t diff = MDIFF(exp1,exp2);

            maxdiff1 = std::max(maxdiff1,d1);
            maxdiff2 = std::max(maxdiff2,d2);
            sd1+=d1;
            sd2+=d2;

            if (d1>d2) {
                d1_g_d2++;
                //tools::debug_print("%d, %d (%d), %d (%d), %.5e ===> %d (%d) ===> %d (%d)", us, ps1, ps2, pd1, pd2, expected, exp1, exp2, d1, d2);
            }

            if (d2>d1) {
                d2_g_d1++;
            }

            if (res!=0) {
                assert(false);
            }
        }

        tools::debug_print("diff(dbl) > diff(int): %d ; diff(int) > diff(dbl): %d", d1_g_d2, d2_g_d1);
        tools::debug_print("diff_mean(dbl) = %f; diff_mean(int) = %f", (double)sd1/(double)MCU_MAXIMUM_TIMER_US, (double)sd2/(double)MCU_MAXIMUM_TIMER_US);
        tools::debug_print("max(diff, dbl) = %u; max(diff, int)=%u", maxdiff1, maxdiff2);

        assert(g_assert_param_count==0);
    }    
}

void test_stm32_timer_params() {
    DECLARE_TEST(test_stm32_timer_params)
    const double max_err = 0.05;

    REPORT_CASE
    {
        uint32_t freq = 8000000; // 8MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 1.0e-6; //1us
        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);
        assert(expected < s*(1.0 + max_err));
        assert(expected > s*(1.0 - max_err));

    }

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 1.0e-6; // 1us
        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);        
        assert(expected < s*(1.0 + max_err));
        assert(expected > s*(1.0 - max_err));
    }    

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 1.0e-9; // 1ns - must fail
        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);
        assert(prescaller==0);
        assert(period==0);
        assert(expected<1.4e-08);
    }        

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 1.0; // 1s
        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);        
        assert(expected < s*(1.0 + max_err));
        assert(expected > s*(1.0 - max_err));
    }      

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 29.0; // 29s
        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);        
        assert(expected < s*(1.0 + max_err));
        assert(expected > s*(1.0 - max_err));
    }          

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 60.0; // 1m

        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==1);
    }    
}

void test_circbuffer_single_byte() {
    DECLARE_TEST(test_circbuffer_single_byte)

    // block mode: block_size==1
    REPORT_CASE
    {
        g_assert_param_count = 0;
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 1;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        refcirc.status = 0;
        refcirc.status_size = 0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(g_assert_param_count==0); // no asserts

        // read a byte (overflow)
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;        
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);

        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);            

        // write byte (1)
        circbuf_put_byte(&circ, 1);

        refbuffer[0] = 1;
        refcirc.put_pos=0;
        refcirc.data_len=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);


        // read byte
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;        
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);

        // write byte
        circbuf_put_byte(&circ, 2);

        refbuffer[0] = 2;
        refcirc.put_pos=0;
        refcirc.data_len=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // write byte (overflow)
        circbuf_put_byte(&circ, 3);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==1);

        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);           

        // read byte
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==2);

        refcirc.bytes_read=1;
        refcirc.read_pos=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;        
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
    }
}

void test_circbuffer_single_block() {
    DECLARE_TEST(test_circbuffer_single_block)

    // block mode: block_size==buffer_size
    REPORT_CASE
    {
        g_assert_param_count = 0;
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 2;
        const uint16_t block_size = 2;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_size=block_size;
        refcirc.free_size=0;
        assert(g_assert_param_count==0); // no asserts
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve, write and commit one block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.current_block = buffer;
        assert(block==buffer);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(circbuf_get_ovf(&circ)==0);

        // write
        block[0] = 1;
        block[1] = 2;
        refbuffer[0] = 1;
        refbuffer[1] = 2;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);                

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size;
        refcirc.put_pos = 0;
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0); 

        // reserve one more block (overflow)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);        
        assert(circbuf_get_ovf(&circ)==1);

        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);        

        // read one byte
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 1;
        refcirc.start_pos = 1;        
        assert(circbuf_len(&circ)==1);

        // reserve one more block (overflow)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(circbuf_get_ovf(&circ)==1);

        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);          

        // read one byte
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==2);

        refcirc.bytes_read=1;
        refcirc.read_pos=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;  
        assert(circbuf_len(&circ)==0);              

        // reserve one more block (success)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.current_block = buffer;
        assert(block==buffer);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(circbuf_get_ovf(&circ)==0);

        // cancel block
        circbuf_cancel_block(&circ);
        refcirc.current_block = 0;
        refcirc.data_len = 0;
        assert(circ.put_pos == 0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);         

        // reserve, write and commit one block (success)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.current_block = buffer;
        assert(block==buffer);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(circbuf_get_ovf(&circ)==0);

        // write
        block[0] = 3;
        block[1] = 4;
        refbuffer[0] = 3;
        refbuffer[1] = 4;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);                

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size;
        refcirc.put_pos = 0;
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0); 

        // read two bytes
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_get_ovf(&circ)==0);

        for (int i=0; i<2; i++) {
            b = 0;
            opres = circbuf_get_byte(&circ, &b);
            assert(opres==1);
            assert(b==3+i);

            refcirc.bytes_read=i+1;
            refcirc.read_pos=(i+1) % block_size;

            assert(memcmp(buffer,refbuffer,buffer_size)==0);
            assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
            assert(circbuf_len(&circ)==2);
            assert(circbuf_get_ovf(&circ)==0);
        }

        // stop reading
        res = circbuf_stop_read(&circ, 2);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;  
        assert(circbuf_len(&circ)==0);

        // read one byte    (overflow)
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);

        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);           

        res = circbuf_stop_read(&circ, 1); // read more bytes than actually read !!!
        refcirc.data_len = 0;
        refcirc.start_pos = 0;  
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
    }
}

void test_circbuffer_asserts() {
    DECLARE_TEST(test_circbuffer_asserts)

    // byte mode: circbuf_reserve_block() must assert
    REPORT_CASE
    {
        g_assert_param_count = 0;
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        refcirc.status = 0;
        refcirc.start_pos = 0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(g_assert_param_count==1); // undefined behaviour, assertion must be triggered
    }

    REPORT_CASE
    {
        g_assert_param_count = 0;
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_commit_block(&circ);
        assert(g_assert_param_count==2); // undefined behaviour, assertion must be triggered (one for not being in block mode, another for unalocated block)
    }    

    REPORT_CASE
    {
        g_assert_param_count = 0;
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_cancel_block(&circ);
        assert(g_assert_param_count==2); // undefined behaviour, assertion must be triggered (one for not being in block mode, another for unalocated block)
    }

    REPORT_CASE // block mode: attempt to init with buffer not multiple by block size
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 9;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==1); // undefined behaviour, assertion must be triggered)
    }    

    REPORT_CASE // block mode: attempt to init with block size == 1
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 9;
        const uint16_t block_size = 1;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==1); // undefined behaviour, assertion must be triggered)
    } 

    REPORT_CASE // block mode: attempt to init with block size > buffer_size
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==2); // undefined behaviour, assertion must be triggered: one time for being non multuple by block_size, another time for buffer size
    }     

    REPORT_CASE // block mode: attempt to init block mode while in block mode
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==0); // must be ok
        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==1); // not ok - already in block mode
    }         

    REPORT_CASE // block mode attempt to call circbuf_put_byte()
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==0); // must be ok
        b = 0;
        circbuf_put_byte(&circ, b);
        assert(g_assert_param_count==1); // not ok - in block mode
    }    

    REPORT_CASE // block mode attempt to call circbuf_commit_block() when block is not allocated
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==0); // must be ok
        circbuf_commit_block(&circ);
        assert(g_assert_param_count==1); // not ok - block was not reserved
    }        

    REPORT_CASE // block mode attempt to call circbuf_cancel_block() when block is not allocated
    {
        g_assert_param_count = 0;        
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==0); // must be ok
        circbuf_cancel_block(&circ);
        assert(g_assert_param_count==1); // not ok - block was not reserved
    }            
}

void test_circbuffer_block_mode_work() {
    DECLARE_TEST(test_circbuffer_block_mode_work)

    REPORT_CASE
    {
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);   
        uint8_t opres,b,res;    

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        refcirc.status = 0;
        refcirc.status_size = 0;

        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_size=block_size;
        refcirc.free_size=buffer_size-block_size;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        block[0] = 1;
        block[1] = 2;
        block[2] = 3;
        block[3] = 4;

        refbuffer[0] = 1;
        refbuffer[1] = 2;
        refbuffer[2] = 3;
        refbuffer[3] = 4;        

        refcirc.current_block = buffer;
        assert(block==buffer);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size;
        refcirc.put_pos = block_size;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);        

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer+block_size;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);        

        // cancel block
        circbuf_cancel_block(&circ);
        refcirc.current_block = 0;
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer+block_size;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        block[0] = 5;
        block[1] = 6;
        block[2] = 7;
        block[3] = 8;

        refbuffer[4] = 5;
        refbuffer[5] = 6;
        refbuffer[6] = 7;
        refbuffer[7] = 8;                

        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size*2;
        refcirc.put_pos = 0;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==8);

        // read 3 bytes (1)
        for (uint8_t i=0; i<4; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            assert(opres==1);
            assert(b==1+i);

            refcirc.bytes_read=1+i;
            refcirc.read_pos=1+i;

            assert(memcmp(buffer,refbuffer,buffer_size)==0);
            assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
        }

        // stop reading
        res = circbuf_stop_read(&circ, 3);
        refcirc.data_len = 5;
        refcirc.start_pos = 3;

        assert(res == 5); // 5 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==5);

        // there are 5 bytes in buffer, we can't reserve new block, test it
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.ovf = 1;
        assert(block==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // read one more byte to free space for new block
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==5);        

        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==4);

        refcirc.bytes_read=1;
        refcirc.read_pos=4;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==5);
        assert(circbuf_get_ovf(&circ)==1);        // flag is still set, we'll clear it later

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 4;
        refcirc.start_pos = 4;

        assert(res == 4); // 4 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==4);

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==4);

        // reserve block again
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // write
        block[0] = 9;
        block[1] = 10;
        block[2] = 11;
        block[3] = 12;

        refbuffer[0] = 9;
        refbuffer[1] = 10;
        refbuffer[2] = 11;
        refbuffer[3] = 12;  

        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size*2;
        refcirc.put_pos = 4;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // prepare for read
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==8);

        // read all the data in the buffer (8 bytes)
        for (uint8_t i=0; i<8; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            assert(opres==1);
            assert(b==5+i);

            refcirc.bytes_read=1+i;
            refcirc.read_pos=5+i;
            refcirc.read_pos=refcirc.read_pos%buffer_size;


            assert(memcmp(buffer,refbuffer,buffer_size)==0);
            assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
        }


        // read one byte from empty buffer
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.bytes_read=8;
        refcirc.read_pos=4;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==1);
        assert(circbuf_len(&circ)==8);
        assert(circbuf_get_ovf(&circ)==1);

        // stop reading
        res = circbuf_stop_read(&circ, 8);
        refcirc.data_len = 0;
        refcirc.start_pos = 4;

        assert(res == 0); // 0 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==1);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);    

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);        

    }
}


void test_circbuffer_block_mode_initialization() {
    DECLARE_TEST(test_circbuffer_block_mode_initialization)

    REPORT_CASE
    {
        // test initialization
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 4;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);        

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_size=block_size;
        refcirc.free_size=buffer_size-block_size;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);        
    }

    REPORT_CASE
    {
        // test initialization with non multiple buffer 
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 17;
        const uint16_t block_size = 4;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);        

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);

        refcirc.block_size=block_size;
        refcirc.buffer_size = (buffer_size / block_size)*block_size;
        refcirc.free_size=refcirc.buffer_size-block_size;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);        
    }    
}

void test_circbuffer_byte_mode() {
    DECLARE_TEST(test_circbuffer_byte_mode)

    REPORT_CASE
    {
        // test initialization
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 10;
        uint8_t buffer[10] = {0};
        circbuf_init(&circ, buffer, sizeof(buffer));        

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
    }

    REPORT_CASE
    {
        // test byte writing
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 10;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        uint8_t b;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;

        circbuf_init(&circ, buffer, buffer_size);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);      
        assert(circbuf_len(&circ)==0);  

        // put a byte (1)
        circbuf_put_byte(&circ, 1);

        refbuffer[0] = 1;
        refcirc.put_pos=1;
        refcirc.data_len=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);


        // read a byte
        b = 0;
        uint8_t opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        uint16_t res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 1;

        assert(res == 0); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE {
        // attempt to read empty buffer
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 10;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        uint8_t b;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;

        circbuf_init(&circ, buffer, buffer_size);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);

        // read a byte
        b = 0;
        uint8_t opres = circbuf_get_byte(&circ, &b);

        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.bytes_read=0;
        refcirc.read_pos=0;
        refcirc.ovf = 1;

        assert(circbuf_get_ovf(&circ)==1);
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==0);

        // stop reading
        uint16_t res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;

        assert(res == 0); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);

        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;        

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE {
        // test buffer overflow
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 2;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        uint8_t b;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;

        circbuf_init(&circ, buffer, buffer_size);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);  
        assert(circbuf_get_ovf(&circ)==0);   
        assert(circbuf_len(&circ)==0);

        // put a byte (1)
        circbuf_put_byte(&circ, 1);

        refbuffer[0] = 1;
        refcirc.put_pos=1;
        refcirc.data_len=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);


        // put a byte (2)
        circbuf_put_byte(&circ, 2);

        refbuffer[1] = 2;
        refcirc.put_pos=0;
        refcirc.data_len=2;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // put a byte (overflow must occur)
        circbuf_put_byte(&circ, 3);

        refcirc.ovf=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==2);

        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);

        // read a byte (1)
        b = 0;
        uint8_t opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // read a byte (2)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==2);

        refcirc.bytes_read=2;
        refcirc.read_pos=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);


        // read a byte (no bytes must be read, buffer is empty)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==2);

        // stop reading 1 byte
        uint8_t res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 1;
        refcirc.start_pos = 1;

        assert(res == 1); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);                
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==1);

        // stop reading another 1 byte (there were 2 in total)
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 0;

        assert(res == 0); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE {
        // test buffer with readin in the middle
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 3;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        uint8_t b, opres;
        uint16_t res;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;

        circbuf_init(&circ, buffer, buffer_size);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);     
        assert(circbuf_len(&circ)==0);

        // put a byte (1)
        circbuf_put_byte(&circ, 1);

        refbuffer[0] = 1;
        refcirc.put_pos=1;
        refcirc.data_len=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==1);


        // put a byte (2)
        circbuf_put_byte(&circ, 2);

        refbuffer[1] = 2;
        refcirc.put_pos=2;
        refcirc.data_len=2;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);

        // put a byte (3)
        circbuf_put_byte(&circ, 3);

        refbuffer[2] = 3;
        refcirc.put_pos=0;
        refcirc.data_len=3;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==3);


        // Read a byte
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer (again, previos read will be discarded)
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_len(&circ)==3);

        // read that byte again
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==1);

        refcirc.bytes_read=1;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==3);

        // stop reading this byte
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 2;
        refcirc.start_pos = 1;

        assert(res == 2); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==2);

        // put a byte (overflow must NOT occur)
        circbuf_put_byte(&circ, 4);

        refbuffer[0]=4;
        refcirc.put_pos=1;
        refcirc.data_len=3;        

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // put a byte (overflow must occur)
        circbuf_put_byte(&circ, 4);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==3);

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer again (next byte will be 2)
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (2)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==2);

        refcirc.bytes_read=1;
        refcirc.read_pos=2;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (3)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==3);

        refcirc.bytes_read=2;
        refcirc.read_pos=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (4)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==1);
        assert(b==4);

        refcirc.bytes_read=3;
        refcirc.read_pos=1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);        
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);


        // read a byte (no bytes must be read, buffer is empty)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);

        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.ovf = 1;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==3);

        // stop reading 5 bytes (more than buffer allows) - buffer will read 3 bytes anyway
        res = circbuf_stop_read(&circ, 5);
        refcirc.data_len = 0;
        refcirc.start_pos = 1;

        assert(res == 0); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);                
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);

        // stop reading another 1 byte (none left)
        res = circbuf_stop_read(&circ, 1);
        refcirc.data_len = 0;
        refcirc.start_pos = 1;

        assert(res == 0); // no bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);
    }    
}

void test_circbuffer_byte_mode_with_status() {
    DECLARE_TEST(test_circbuffer_byte_mode_with_status)

    REPORT_CASE {
        CircBuffer circ;
        uint8_t status = 0xDA;
        uint8_t b = 0xFF;
        uint8_t res;
        uint16_t left = 0;
        const uint16_t buffer_size = 10;
        uint8_t buffer[10] = {0};
        circbuf_init(&circ, buffer, sizeof(buffer));
        circbuf_init_status(&circ, &status, 1);

        circbuf_put_byte(&circ, 42);
        circbuf_put_byte(&circ, 43);

        circbuf_start_read(&circ);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==42);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==43);

        res = circbuf_get_byte(&circ, &b);
        assert(res==0);
        assert(circbuf_get_ovf(&circ) == 1);
        assert(b==COMM_BAD_BYTE);

        circbuf_clear_ovf(&circ);

        left = circbuf_stop_read(&circ, 4);
        assert(left == 0);
    }

    REPORT_CASE {
        CircBuffer circ;
        uint8_t status = 0xDA;
        uint8_t b = 0xFF;
        uint8_t res;
        uint16_t left = 0;
        const uint16_t buffer_size = 10;
        uint8_t buffer[3] = {0};
        circbuf_init(&circ, buffer, sizeof(buffer));
        circbuf_init_status(&circ, &status, 1);

        circbuf_put_byte(&circ, 42);
        circbuf_put_byte(&circ, 43);

        circbuf_start_read(&circ);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==42);

        left = circbuf_stop_read(&circ, 2);
        assert(left == 1);

        circbuf_start_read(&circ);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==43);

        left = circbuf_stop_read(&circ, 2);
        assert(left == 0);

        circbuf_put_byte(&circ, 44);
        circbuf_put_byte(&circ, 45);

        circbuf_start_read(&circ);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==44);

        res = circbuf_get_byte(&circ, &b);
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==45);

        left = circbuf_stop_read(&circ, 3);
        assert(left == 0);
    }
}

void test_circbuffer_block_mode_work_with_status() {
    DECLARE_TEST(test_circbuffer_block_mode_work_with_status)

    REPORT_CASE
    {
        CircBuffer circ;
        CircBuffer refcirc;
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t status = 0xDA;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_status(&circ, &status, 1);
        uint8_t opres,b,res;
        uint16_t res16;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.put_pos=0;
        refcirc.start_pos=0;
        refcirc.data_len=0;
        refcirc.read_pos=0;
        refcirc.bytes_read=0;
        refcirc.ovf=0;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.current_block=0;
        refcirc.status = &status;
        refcirc.status_size = 1;

        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_size=block_size;
        refcirc.free_size=buffer_size-block_size;
        assert(circbuf_len(&circ)==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        block[0] = 1;
        block[1] = 2;
        block[2] = 3;
        block[3] = 4;

        refbuffer[0] = 1;
        refbuffer[1] = 2;
        refbuffer[2] = 3;
        refbuffer[3] = 4;

        refcirc.current_block = buffer;
        assert(block==buffer);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);
        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size;
        refcirc.put_pos = block_size;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer+block_size;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // cancel block
        circbuf_cancel_block(&circ);
        refcirc.current_block = 0;
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer+block_size;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        block[0] = 5;
        block[1] = 6;
        block[2] = 7;
        block[3] = 8;

        refbuffer[4] = 5;
        refbuffer[5] = 6;
        refbuffer[6] = 7;
        refbuffer[7] = 8;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size*2;
        refcirc.put_pos = 0;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==8);

        res16 = circbuf_get_byte(&circ, &b);
        assert(b==0xDA);
        refcirc.bytes_read = 1;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);

        // read 3 bytes (1)
        for (uint8_t i=0; i<4; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            assert(opres==1);
            assert(b==1+i);

            refcirc.bytes_read=2+i;
            refcirc.read_pos=1+i;

            assert(memcmp(buffer,refbuffer,buffer_size)==0);
            assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
        }

        // stop reading
        res = circbuf_stop_read(&circ, 4);
        refcirc.data_len = 5;
        refcirc.start_pos = 3;

        assert(res == 5); // 5 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==5);

        // there are 5 bytes in buffer, we can't reserve new block, test it
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.ovf = 1;
        assert(block==0);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // read one more byte to free space for new block
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==5);

        opres = circbuf_get_byte(&circ, &b);
        assert(b==0xDA);
        assert(opres==1);
        refcirc.bytes_read=1;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);

        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==4);

        refcirc.bytes_read=2;
        refcirc.read_pos=4;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==5);
        assert(circbuf_get_ovf(&circ)==1);        // flag is still set, we'll clear it later

        // stop reading
        res = circbuf_stop_read(&circ, 2);
        refcirc.data_len = 4;
        refcirc.start_pos = 4;

        assert(res == 4); // 4 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==4);

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==4);

        // reserve block again
        block = (uint8_t*)circbuf_reserve_block(&circ);

        refcirc.current_block = buffer;
        assert(block==refcirc.current_block);
        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // write
        block[0] = 9;
        block[1] = 10;
        block[2] = 11;
        block[3] = 12;

        refbuffer[0] = 9;
        refbuffer[1] = 10;
        refbuffer[2] = 11;
        refbuffer[3] = 12;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);

        // commit block
        circbuf_commit_block(&circ);

        refcirc.current_block = 0;
        refcirc.data_len = block_size*2;
        refcirc.put_pos = 4;

        assert(memcmp(&circ,&refcirc,sizeof(CircBuffer))==0);

        // prepare for read
        circbuf_start_read(&circ);
        refcirc.read_pos = refcirc.start_pos;
        refcirc.bytes_read = 0;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_len(&circ)==8);

        opres = circbuf_get_byte(&circ, &b);
        assert(b==0xDA);
        refcirc.bytes_read=1;
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);

        // read all the data in the buffer (8 bytes)
        for (uint8_t i=0; i<8; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            assert(opres==1);
            assert(b==5+i);

            refcirc.bytes_read=2+i;
            refcirc.read_pos=5+i;
            refcirc.read_pos=refcirc.read_pos%buffer_size;


            assert(memcmp(buffer,refbuffer,buffer_size)==0);
            assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
        }


        // read one byte from empty buffer
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);

        refcirc.bytes_read=8;
        refcirc.read_pos=4;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==1);
        assert(circbuf_len(&circ)==8);
        assert(circbuf_get_ovf(&circ)==1);

        // stop reading
        res = circbuf_stop_read(&circ, 9);
        refcirc.data_len = 0;
        refcirc.start_pos = 4;

        assert(res == 0); // 0 bytes remains
        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==1);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf=0;

        assert(memcmp(buffer,refbuffer,buffer_size)==0);
        assert(memcmp(&circ,&refcirc,sizeof(refcirc))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);

    }
}

void test_safe_mutex() {
    DECLARE_TEST(test_safe_mutex)
    REPORT_CASE
    {
        tools::safe_mutex a;
        a.lock();
        CHECK_SAFE_MUTEX_LOCKED(a);
        a.unlock();
    }

    REPORT_CASE
    {
        tools::safe_mutex a;
        tools::safe_mutex b;
        tools::safe_mutex c;
        a.lock();
        CHECK_SAFE_MUTEX_LOCKED(a);
        b.lock();
        CHECK_SAFE_MUTEX_LOCKED(b);
        c.lock();
        CHECK_SAFE_MUTEX_LOCKED(c);

        c.unlock();        
        b.unlock();
        a.unlock();        
    }
}

void test_icu_regex_group() {
    DECLARE_TEST(test_parse_command)

    std::string s;
    std::unique_ptr<RegexPattern> re;
    std::vector<std::string> groups;
    bool res;

    REPORT_CASE
    s = "+CUSD: 1,\"0031\", 72";
    re = tools::g_unicode_ts.regex_pattern("\\+CUSD:\\s?(\\d+)\\s?,\\s?\\\"([^\\\"]*)\\\"\\s?,\\s?(\\d+)", 0);
    assert(re);
    groups.clear();
    res = tools::g_unicode_ts.regex_groups(*re, s, groups);
    assert(res);
    assert(groups.size()==4);
    assert(groups[0].compare(s)==0);
    assert(groups[1].compare("1")==0);
    assert(groups[2].compare("0031")==0);
    assert(groups[3].compare("72")==0);

    REPORT_CASE
    s = "+CMGL: 59,\"REC READ\",\"002B\",\"\",\"20/08/06,16:29:57+12\"";
    re = tools::g_unicode_ts.regex_pattern("\\+CMGL:\\s*(\\d+)\\s*,\\s*\\\"([^\\\"\\d]+)\\\"\\s*,\\s*\\\"([a-fA-F\\d]+)\\\"\\s*,\\s*\\\"([^\\\"]*)\\\"\\s*,\\s*\\\"(\\S+)\\\"", 0);
    assert(re);
    groups.clear();
    res = tools::g_unicode_ts.regex_groups(*re, s, groups);
    assert(res);
    assert(groups.size()==6);
    assert(groups[0].compare(s)==0);
    assert(groups[1].compare("59")==0);
    assert(groups[2].compare("REC READ")==0);
    assert(groups[3].compare("002B")==0);
    assert(groups[4].compare("")==0);
    assert(groups[5].compare("20/08/06,16:29:57+12")==0);
}

void test_check_prefix() {
    DECLARE_TEST(test_check_prefix)
    bool result;

    REPORT_CASE
    result = tools::check_prefix((const char*)"",(const char*)"");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("","A");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("","AA");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("A","");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("A","A");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("AA","A");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("C","CC");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("CC","CC");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("CCC","CC");
    assert(result==true);
}

void test_StopWatch() {
    DECLARE_TEST(test_StopWatch)
    bool expired;
    size_t result;

    REPORT_CASE
    tools::StopWatch<std::chrono::milliseconds> ms_sw(1500);
    tools::sleep_ms(1000);
    result = ms_sw.measure();
    expired = ms_sw.expired();
    assert(expired==false);
    assert(result>=1000 && result<=1100);

    REPORT_CASE
    tools::sleep_ms(1000);
    result = ms_sw.measure();
    assert(result>=2000 && result<=2200);
    expired = ms_sw.expired();
    assert(expired==true);


    REPORT_CASE
    ms_sw.restart();
    tools::sleep_ms(1000);
    result = ms_sw.measure();
    assert(result>=1000 && result<=1100);
    expired = ms_sw.expired();
    assert(expired==false);
    ms_sw.pause();
    tools::sleep_ms(1000);
    expired = ms_sw.expired();
    assert(expired==false);
    ms_sw.resume();
    expired = ms_sw.expired();
    assert(expired==false);
    tools::sleep_ms(1000);
    expired = ms_sw.expired();
    assert(expired==true);
}


void test_append_vector() {
    DECLARE_TEST(test_append_vector)

    std::vector<int> ires;

    REPORT_CASE
    std::vector<int> v1 = {0,1,2,3};
    std::vector<int> v2 = {4,5,6,7};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2,3,4,5,6,7}));

    REPORT_CASE
    v1 = {0};
    v2 = {1};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1}));

    REPORT_CASE
    v1 = {};
    v2 = {1};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({1}));

    REPORT_CASE
    v1 = {0};
    v2 = {};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0}));

    REPORT_CASE
    v1 = {0,1};
    v2 = {2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));

    REPORT_CASE
    v1 = {0};
    v2 = {1,2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));

    REPORT_CASE
    v1 = {};
    v2 = {0,1,2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));    

    REPORT_CASE
    v1 = {0,1,2};
    v2 = {};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));    
}

void test_hex_val() {
    DECLARE_TEST(test_hex_val)
    REPORT_CASE

    for (size_t i=0; i<256; ++i) {
        char c = (char) (i & 0xFF);
        uint8_t res, exp;

        res = tools::SpecialCharacterTables::hex_val[i];

        if (c>='0' && c<='9') {
            exp = c - '0';
        } else if (c>='A' && c<='F') {
            exp = 0x0A + (c - 'A');
        } else if (c>='a' && c<='f') {
            exp = 0x0A + (c - 'a');
        } else {
            exp = 255;
        }

        assert(exp==res);
    }
}

void test_buffer_to_hex() {
    DECLARE_TEST(test_hex_val)

    REPORT_CASE
    std::vector<uint8_t> buffer = {0x00};
    std::string res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, nullptr);
    assert(res.compare("00")==0);

    REPORT_CASE
    buffer = {};
    res = tools::buffer_to_hex(nullptr,  buffer.size(), true, nullptr);
    assert(res.compare("")==0);

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, nullptr);
    assert(res.compare("0102")==0);

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, "");
    assert(res.compare("0102")==0);    

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, "->");
    assert(res.compare("01->02")==0);

    REPORT_CASE
    buffer = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), false, " ");
    assert(res.compare("01 23 45 67 89 AB CD EF")==0);    

    REPORT_CASE
    buffer = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, " ");
    assert(res.compare("01 23 45 67 89 ab cd ef")==0);
}

void test_buffer_from_hex() {
    DECLARE_TEST(test_buffer_from_hex)

    REPORT_CASE
    std::string hex = "";
    std::vector<uint8_t> exp;
    std::vector<uint8_t> res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "000102";
    exp = {0x00, 0x01, 0x02};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "00";
    exp = {0x00};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));    

    REPORT_CASE
    hex = "0123456789ABCDEF";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));        

    REPORT_CASE
    hex = "0123456789abcdef";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));        

    REPORT_CASE
    hex = "0123456789abCDeF";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));            

    REPORT_CASE
    try {
        hex = "01234567P9abCDeF";
        exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        res = tools::buffer_from_hex(hex);
        assert(false);
    } catch (std::out_of_range& e) {
    }

    REPORT_CASE
    try {
        hex = "01234567P9abCDe";
        exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        res = tools::buffer_from_hex(hex);
        assert(std::equal(exp.begin(), exp.end(), res.begin()));
    } catch (std::length_error& e) {
    }    
}

void test_split_and_trim() {
    DECLARE_TEST(split_and_trim)
    std::vector<std::string> lines;
    std::string s;

    REPORT_CASE
    s = "";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = "1";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("1")==0);

    REPORT_CASE
    s = "11";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);    

    REPORT_CASE
    s = "11\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);        

    REPORT_CASE
    s = "\n11\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);            

    REPORT_CASE
    s = "\n11\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);                

    REPORT_CASE
    s = "\n\n11\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);                    

    REPORT_CASE
    s = "\n\n1 1\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("1 1")==0);                        

    REPORT_CASE
    s = "\n\n11 \n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);                        

    REPORT_CASE
    s = "\n\n11 \n\n1";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);

    REPORT_CASE
    s = "\n\n11 \n\n1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);    

    REPORT_CASE
    s = "\n\n11 \n\n 1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);        

    REPORT_CASE
    s = "\n\n 11 \n\n 1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);        

    REPORT_CASE
    s = "\n\n 11 \n\n    ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = " \n\n  \n \n    \n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = " \n\n  \n \n    \n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = "\n \n\n  \n \n    \n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);
}


void test_trim_string() {
    DECLARE_TEST(test_trim_string)
    size_t res = 0;
    std::string s;

    REPORT_CASE
    s = "";
    res = tools::trim_string(s, [](char b){return false;});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = " ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = " ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "      ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "1";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = "1 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = " 1";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = " 1 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = "12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = " 12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "12 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = " 12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12  ";
    res = tools::trim_string(s, [](char b){return std::isspace(b);});
    assert(res==2);
    assert(s.compare("12")==0);    
}



int main()
{
    std::cout << "******************************************* TESTTOOL *******************************************" << std::endl;
    test_stm32_timer_params_integer();
    test_stm32_timer_params();
    test_circbuffer_single_byte();
    test_circbuffer_single_block();
    test_circbuffer_asserts();
    test_circbuffer_block_mode_work();
    test_circbuffer_block_mode_initialization();
    test_circbuffer_byte_mode();
    test_circbuffer_byte_mode_with_status();
    test_safe_mutex();
    test_icu_regex_group();
    test_check_prefix();
    test_append_vector();
    test_split_and_trim();
    test_trim_string();
    test_buffer_to_hex();
    test_buffer_from_hex();
    test_hex_val();
    test_StopWatch();
    return 0;
}
