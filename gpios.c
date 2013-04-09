/*
 * Module gpios.c is used to inicialize and handle stellaris launchpad leds and buttons.
 *
 * date: 15.3.2013
 * author: ohejda
 */


#include "utils.h"
#include "gpios.h"

/**
 * The function enables port run-time clock
 * input arg is just the port number
 * PORTA, PORTB .. PORTF defines can be used
 * example: enable port F .. enable_port_clock(PORTF)
 */
void enable_port_clock(int port)
{
	// enable PORT F GPIO peripheral using
	// run-time clock gating control register 2
	SYSCTL_RCGCGPIO_R |= (1<<port);
	// The datasheet says wait after enabling GPIO
	busy_sleep(10);
}

/**
 * The function initializes launchpad LEDs and buttons.
 * It can be handled than by
 * 		BUTTON_PRESSED(x) with BUTTON_ONE and BUTTON_TWO argument
 * 		LED_ON/OFF/SWAP(x) with LED_RED/GREEN/BLUE argument
 * The macros can be found in gpios.h header file
 */
void init_gpios(void)
{
    // enable PORT F GPIO peripheral using
    // run-time clock gating control register 2
    /*SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    // The datasheet says wait after enabling GPIO
    busy_sleep(10);*/
	enable_port_clock(PORTF);

    /* The GPIO for button two is multiplexed with NMI so we
     * have to 'unlock' it before we can use it
     */
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
    GPIO_PORTF_CR_R |= STELLARIS_BUTTON_TWO; /* Allow F0 to be changed */
    GPIO_PORTF_LOCK_R = 0; /* Lock CR again */

    // enable digital for button pins
    GPIO_PORTF_DEN_R |= STELLARIS_BUTTON_ONE | STELLARIS_BUTTON_TWO;

    // set pins to inputs
    GPIO_PORTF_DIR_R &= ~(STELLARIS_BUTTON_ONE | STELLARIS_BUTTON_TWO);

    // Enable weak pullups
    GPIO_PORTF_DR2R_R |= STELLARIS_BUTTON_ONE | STELLARIS_BUTTON_TWO;
    GPIO_PORTF_PUR_R |= STELLARIS_BUTTON_ONE | STELLARIS_BUTTON_TWO;

    // enable digital for LED PORT F pins
    GPIO_PORTF_DEN_R |= LED_RED | LED_BLUE | LED_GREEN;
    // set LED PORT F pins as outputs (rest are inputs)
    GPIO_PORTF_DIR_R |= LED_RED | LED_BLUE | LED_GREEN;

    // user buttons initialization
    enable_port_clock(PORTA);
    GPIO_PORTA_DEN_R  |=  (USER_BUTTON_1 | USER_BUTTON_2 | USER_BUTTON_3);
    GPIO_PORTA_DIR_R  &= ~(USER_BUTTON_1 | USER_BUTTON_2 | USER_BUTTON_3);
    GPIO_PORTA_DR2R_R |=  (USER_BUTTON_1 | USER_BUTTON_2 | USER_BUTTON_3);
    GPIO_PORTA_PUR_R  |=  (USER_BUTTON_1 | USER_BUTTON_2 | USER_BUTTON_3);
}
