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

#include <inttypes.h>

#include "lm4f120h5qr.h"
#include "gpios.h"
#include "utils.h"


/***** GPIOS *****/



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
