#include "timer_tests.hpp"
#include "utools.h"

void test_stm32_timer_params_integer() {
    DECLARE_TEST(test_stm32_timer_params_integer)
    uint32_t us;
    uint16_t prescaller, period;

    REPORT_CASE
    {
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

    REPORT_CASE
    {
        uint32_t freq = 72000000; // 72MHz
        uint16_t prescaller = 0;
        uint16_t period = 0;
        double expected = 0.0;
        double s = 1.0e-6; // 1us

        int res = tools::stm32_timer_params(freq, s, prescaller, period, expected);

        assert(res==0);
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
    }
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