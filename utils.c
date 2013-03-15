/**
 * Module utils.c contains some timing functions for lm4f120h
 * stellaris launchpad board like busysleep and systick
 *
 * autror: ondrejh
 * date: 15.3.2013
 */

#include "utils.h"

/**
 * Rough and ready sleep function. Approx 8 clock ticks elapse
 * per 'delay', although compiler optimisations might
 * massively reduce this.
 */
void busy_sleep(unsigned long delay)
{
    while(delay--)
    {
        /* Spin */
    }
}

/**
 * The function initializes systick timer with tickcnt as the reload value.
 * "SysTick" handler can be triggered by polling SYSTICK_COUNT() macro (see .h file).
 * There are some SYSTICK_[time]_[cpuspeed] macros in .h file which can be used
 * as the tickcnt value.
 */
void init_systick(uint32_t tickcnt)
{
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC; // select system clock clock source
	NVIC_ST_RELOAD_R = tickcnt; // set reload value
	NVIC_ST_CURRENT_R = 0xBAFBAF; // clear current reg. value by writing whatever
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_ENABLE; // run it by setting enable bit
}
