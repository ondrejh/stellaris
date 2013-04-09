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

#define BUTTON_FILTER_TICKS 3

bool btn[3] = {false, false, false};

// filtering user buttons
void user_button_filter(void)
{
	static int btn_filter[3] = {0, 0, 0};
	int i;

	for (i=0;i<3;i++)
	{
		if (((btn[i])&(!USER_BUTTON_PRESSED(USER_BUTTON_1<<i))) || ((!btn[i])&(USER_BUTTON_PRESSED(USER_BUTTON_1<<i))))
		{
			btn_filter[i]++;
			if (btn_filter[i] >= BUTTON_FILTER_TICKS) {btn[i]=!btn[i];}
		}
		else btn_filter[i] = 0;
	}
}

/***** MAIN *****/

void main(void)
{
	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);
	init_timer();
	init_uart0();
	init_uart1();

	init_lcd();

	lcd_prints_buf(0, "Sbohem, A dik za");
	lcd_prints_buf(16,"vsechny ty ryby!");

	while(1)
	{
		// test buttons filtering
		if (btn[0]) {LED_ON(LED_RED);} else {LED_OFF(LED_RED);}
		if (btn[1]) {LED_ON(LED_GREEN);} else {LED_OFF(LED_GREEN);}
		if (btn[2]) {LED_ON(LED_BLUE);} else {LED_OFF(LED_BLUE);}

		char c;
		if (uart1_getchar(&c)) uart1_putchar(c);

		if (SYSTICK_COUNT()) // 1ms timing
		{
			lcd_refresh();

			user_button_filter();

			static int bluetick = 0;
			bluetick++;

			if (bluetick==990)
			{
				//LED_ON(LED_BLUE);
				/*char c;
				while (uart1_getchar(&c)) uart1_putchar(c);*/
			}
			else if (bluetick==1000)
			{
				bluetick=0;
				//LED_OFF(LED_BLUE);
				// uart 0 echo with 1s delay
				char c;
				while (uart0_getchar(&c)) uart0_putchar(c);
			}
		}
	}
}
