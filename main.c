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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "lm4f120h5qr.h"
#include "gpios.h"
#include "utils.h"
#include "lcd.h"


/***** UART *****/

#define RCGCGPIO 0x400FE608
#define PRGPIO   0x400FEA08

#define CM4_BASE 0xE000E000
#define STCTRL      (CM4_BASE+0x010)
#define STRELOAD    (CM4_BASE+0x014)
#define STCURRENT   (CM4_BASE+0x018)

#define RCGCUART 0x400FE618
#define PRUART   0x400FEA18

#define GPIOA_BASE 0x40004000
#define GPIOAFSEL (GPIOA_BASE+0x420)
#define GPIODEN   (GPIOA_BASE+0x51C)

#define UART0_BASE 0x4000C000
#define UARTDR   (UART0_BASE+0x000)
#define UARTFR   (UART0_BASE+0x018)
#define UARTIBRD (UART0_BASE+0x024)
#define UARTFBRD (UART0_BASE+0x028)
#define UARTLCRH (UART0_BASE+0x02C)
#define UARTCTL  (UART0_BASE+0x030)
#define UARTCC   (UART0_BASE+0xFC8)

void uart_init ( void )
{
    //unsigned int ra;
    //16000000 / (16 * 115200 ) = 1000000 / 115200 = 8.6805
    //0.6805... * 64 = 43.5   ~ 44
    //use 8 and 44
    /*ra=GET32(RCGCUART);
    ra|=1; //UART0
    PUT32(RCGCUART,ra);*/
    SYSCTL_RCGCUART_R |=1; // UART0
    /*ra=GET32(RCGCGPIO);
    ra|=1; //GPIOA
    PUT32(RCGCGPIO,ra);*/
    SYSCTL_RCGCGPIO_R |= 1; // GPIOA


    /*ra=GET32(GPIOAFSEL);
    ra|=3;
    PUT32(GPIOAFSEL,ra);*/
    GPIO_PORTA_AFSEL_R |= 3;

    /*ra=GET32(GPIODEN);
    ra|=3;
    PUT32(GPIODEN,ra);*/
    GPIO_PORTA_DEN_R |= 3;

    //PUT32(UARTCTL,0x00000000);
    UART0_CTL_R = 0x00000000;
    //PUT32(UARTIBRD, 8);
    UART0_IBRD_R = 8;
    //PUT32(UARTFBRD, 44);
    UART0_FBRD_R = 44;
    //PUT32(UARTLCRH,0x00000060);
    UART0_LCRH_R = UART_LCRH_WLEN_8;
    //PUT32(UARTCC,0x00000005); //PIOSC
    UART0_CC_R = UART_CC_CS_PIOSC;
    //PUT32(UARTFR,0);
    UART0_FR_R = 0;
    //PUT32(UARTCTL,0x00000301);
    UART0_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);
}


/***** MAIN *****/

void main(void)
{
	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);
	init_timer();

	init_lcd();

	lcd_prints_buf(0, "Nashle, a dik za");
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
			}
		}
	}
}
