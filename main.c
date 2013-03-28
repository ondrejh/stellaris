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
 *   connecting 2x16 char LCD
 *   basic uart functionality
 *
 * more comming soon ...
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "lm4f120h5qr.h"
#include "gpios.h"
#include "utils.h"
#include "lcd.h"
#include "uart.h"


/***** MAIN *****/

void main(void)
{
	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);
	init_timer();
	init_uart0();

	init_lcd();

	lcd_prints_buf(0, "Sbohem, A dik za");
	lcd_prints_buf(16,"vsechny ty ryby!");

	while(1)
	{
		if (BUTTON_PRESSED(BUTTON_ONE)) LED_ON(LED_RED)
		else LED_OFF(LED_RED);
		if (BUTTON_PRESSED(BUTTON_TWO)) LED_ON(LED_GREEN)
		else LED_OFF(LED_GREEN);

		if (SYSTICK_COUNT()) // 1ms timing
		{
			lcd_refresh();

			static int bluetick = 0;
			bluetick++;

			if (bluetick==990)
			{
				LED_ON(LED_BLUE);
			}
			else if (bluetick==1000)
			{
				bluetick=0;
				LED_OFF(LED_BLUE);
				// uart 0 echo with 1s delay
				char c;
				while (uart0_getchar(&c)) uart0_putchar(c);
			}
		}
	}
}
