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
        // Spin
    }
}

/***** TIMEOUT TIMER (us timing)*****/

// define stolen from "driverlib/sysctl.h"
#define SYSCTL_PERIPH_TIMER0  0x10100001 // Timer 0
// defines stolen from "driverlib/rom.h"
#define ROM_APITABLE ((unsigned long *)0x01000010)
#define ROM_SYSCTLTABLE ((unsigned long *)(ROM_APITABLE[13]))
#define ROM_SysCtlPeripheralEnable ((void (*)(unsigned long ulPeripheral))ROM_SYSCTLTABLE[6])

/**
 * Initialization of free running timer used for timeout timers
 */
void init_timer(void)
{
	// enable the peripheral
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	// disable the timer
	TIMER0_CTL_R &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);

	TIMER0_CFG_R = 0x0; // 32 bit mode (no RTC)
	TIMER0_TAMR_R = TIMER_TAMR_TACDIR | TIMER_TAMR_TAMR_PERIOD;// up, periodic
	TIMER0_CTL_R = TIMER_CTL_TAEN; // enable TA
}

#define TIMER_VALUE TIMER0_TAV_R;

// start timer with setting timeout time
void timer_start(timer_t *t, uint32_t timeout)
{
	t->start_time = TIMER_VALUE;
	t->timeout = timeout;
	//t->now = TIMER_VALUE;
}

// test if timeout happens
bool timer_timeout(timer_t *t)
{
	t->now = TIMER_VALUE;
	if ((t->now-t->start_time)>=t->timeout) return true;
	return false;
}

void timer_us_busysleep(timer_t *t, uint32_t timeout)
{
	timer_start(t,timeout);
	while (!timer_timeout(t)) {};
}

/***** SYSTICK TIMER (1ms)*****/

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
