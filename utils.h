/**
 * utils.h
 *
 * author: ondrejh
 * date: 15.3.2013
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <inttypes.h>
#include <stdbool.h>

#include "lm4f120h5qr.h"

void busy_sleep(unsigned long delay);


/***** SYSTICK TIMER *****/

// returns true if systick count occurs
#define SYSTICK_COUNT() (NVIC_ST_CTRL_R&NVIC_ST_CTRL_COUNT)

// systick predefines
#define SYSTICK_1S_64MHZ (16000000-1)
#define SYSTICK_1MS_64MHZ (16000-1)

// function prototypes
void init_systick(uint32_t tickcnt);


/***** TIMEOUT TIMER *****/

// timeout timer context
typedef struct
{
	uint32_t start_time;
	uint32_t timeout;
	uint32_t now;
} timer_t;

// convert microseconds to ticks (multiple by 16 in fact)
#define US_2_TICKS(x) (x*16)

// function prototypes
void init_timer(void);
void timer_start(timer_t *t, uint32_t timeout);
bool timer_timeout(timer_t *t);
void timer_busysleep(timer_t *t, uint32_t timeout);

#endif /* UTILS_H_ */
