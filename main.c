/**
 * This should be test code for stellaris launchpad board.
 * It doesn't need whole huge StellarisWare,
 * but "lm4f120h5qr.h" mcu registers definition file (included) and
 * "startup_css.c" file making it run without debug reseting.
 * It's inspired by several similar GitHub projects.
 *
 * now it shows:
 *   howto drive leds
 *   howto use buttons
 *   howto use systick module for timing
 *
 * more comming soon ...
 */

#include "lm4f120h5qr.h"

#include <inttypes.h>

/***** UTILS *****/

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
 * initializes systick timer with tickcnt as the reload value
 * "SysTick" handler can be triggered by polling STCTRL COUNT bit
 */
void init_systick(uint32_t tickcnt)
{
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC; // select system clock clock source
	NVIC_ST_RELOAD_R = tickcnt; // set reload value
	NVIC_ST_CURRENT_R = 0xBAFBAF; // clear current reg. value by writing whatever
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_ENABLE; // run it by setting enable bit
}

// returns true if systick count occurs
#define SYSTICK_COUNT() (NVIC_ST_CTRL_R&NVIC_ST_CTRL_COUNT)

// systick predefines
#define SYSTICK_1S_64MHZ (16000000-1)
#define SYSTICK_1MS_64MHZ (16000-1)


/***** GPIOS *****/

/* The LEDs on the Launchpad board are pins GPIO_F1..3 */
#define LED_RED (1<<1)
#define LED_BLUE (1<<2)
#define LED_GREEN (1<<3)

/* The buttons are pins GPIO_F0 and GPIO_F4*/
#define BUTTON_ONE (1<<4)
#define BUTTON_TWO (1<<0)

#define BUTTON_PRESSED(x) ((GPIO_PORTF_DATA_R&x)==0)
#define LED_ON(x) {GPIO_PORTF_DATA_R|=x;}
#define LED_OFF(x) {GPIO_PORTF_DATA_R&=~x;}
#define LED_SWAP(x)	{GPIO_PORTF_DATA_R^=x;}

void init_gpios(void)
{
    // enable PORT F GPIO peripheral using
    // run-time clock gating control register 2
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    // The datasheet says wait after enabling GPIO
    busy_sleep(10);

    /* The GPIO for button two is multiplexed with NMI so we
     * have to 'unlock' it before we can use it
     */
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
    GPIO_PORTF_CR_R |= BUTTON_TWO; /* Allow F0 to be changed */
    GPIO_PORTF_LOCK_R = 0; /* Lock CR again */

    // enable digital for button pins
    GPIO_PORTF_DEN_R |= BUTTON_ONE | BUTTON_TWO;

    // set pins to inputs
    GPIO_PORTF_DIR_R &= (BUTTON_ONE | BUTTON_TWO);

    // Enable weak pullups
    GPIO_PORTF_DR2R_R |= BUTTON_ONE | BUTTON_TWO;
    GPIO_PORTF_PUR_R |= BUTTON_ONE | BUTTON_TWO;

    // enable digital for LED PORT F pins
    GPIO_PORTF_DEN_R |= LED_RED | LED_BLUE | LED_GREEN;
    // set LED PORT F pins as outputs (rest are inputs)
    GPIO_PORTF_DIR_R |= LED_RED | LED_BLUE | LED_GREEN;
}


/***** MAIN *****/

void main(void)
{
	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);

	while(1)
	{
		if (BUTTON_PRESSED(BUTTON_ONE)) LED_ON(LED_RED)
		else LED_OFF(LED_RED);
		if (BUTTON_PRESSED(BUTTON_TWO)) LED_ON(LED_GREEN)
		else LED_OFF(LED_GREEN);
		if (SYSTICK_COUNT())
		{
			static int bluetick = 0;
			bluetick++;

			if (bluetick==990) LED_ON(LED_BLUE)
			else if (bluetick==1000)
			{
				bluetick=0;
				LED_OFF(LED_BLUE);
			}

		}
	}
}
