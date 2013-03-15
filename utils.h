/**
 * utils.h
 *
 * author: ondrejh
 * date: 15.3.2013
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <inttypes.h>
#include "lm4f120h5qr.h"

/// systick macros

// returns true if systick count occurs
#define SYSTICK_COUNT() (NVIC_ST_CTRL_R&NVIC_ST_CTRL_COUNT)

// systick predefines
#define SYSTICK_1S_64MHZ (16000000-1)
#define SYSTICK_1MS_64MHZ (16000-1)

// function prototypes
void busy_sleep(unsigned long delay);
void init_systick(uint32_t tickcnt);

#endif /* UTILS_H_ */
