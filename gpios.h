/*
 * gpios.h
 *
 * author: ohejda
 * date: 15.3.2013
 */

#ifndef GPIOS_H_
#define GPIOS_H_


// module global includes
#include <inttypes.h>
#include "lm4f120h5qr.h"

// list of ports
#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5

// The LEDs on the Launchpad board are pins GPIO_F1..3
#define LED_RED (1<<1)
#define LED_BLUE (1<<2)
#define LED_GREEN (1<<3)

// The buttons are pins GPIO_F0 and GPIO_F4
#define BUTTON_ONE (1<<4)
#define BUTTON_TWO (1<<0)

#define BUTTON_PRESSED(x) ((GPIO_PORTF_DATA_R&x)==0)
#define LED_ON(x) {GPIO_PORTF_DATA_R|=x;}
#define LED_OFF(x) {GPIO_PORTF_DATA_R&=~x;}
#define LED_SWAP(x)	{GPIO_PORTF_DATA_R^=x;}


// function prototypes
void init_gpios(void);

// function enabling port runtime clock (port number given by argument)
void enable_port_clock(int port);

#endif /* GPIOS_H_ */
