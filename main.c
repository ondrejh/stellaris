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


/***** LCD *****/

#define LCD_RSPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x04;}
#define LCD_RSPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x04;}
#define LCD_RWPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x08;}
#define LCD_RWPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x08;}
#define LCD_ENPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x10;}
#define LCD_ENPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x10;}

#define LCD_RESET_ALL_PINS() {GPIO_PORTA_DATA_R&=~0x1C;GPIO_PORTC_DATA_R&=~0xF0;}

#define LCD_PULSE_DELAY (10)
//#define LCD_INTERCMD_DELAY (100)
#define LCD_INIT_DELAY (500000)
#define LCD_STARTUP_DELAY (1000000)

timer_t tlcd;

void lcd_set_dataport(uint8_t data)
{
	GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~0xF0) | (data&0xF0);
	GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & ~0xC0) | ((data&0x0C)<<4);
	GPIO_PORTD_DATA_R = (GPIO_PORTD_DATA_R & ~0xC0) | ((data&0x03)<<6);
}

void lcd_pulse_data(uint8_t data, uint8_t cnd)
{
	// set data
	lcd_set_dataport(data);
	// set control nibbles
	LCD_RWPIN_SET_LOW(); // write
	if (cnd==0) LCD_RSPIN_SET_HIGH() else LCD_RSPIN_SET_LOW(); // command or data?

	LCD_ENPIN_SET_LOW(); // en pin low
	timer_busysleep(&tlcd, US_2_TICKS(1));
	LCD_ENPIN_SET_HIGH(); // en pin high
	timer_busysleep(&tlcd, US_2_TICKS(1));
	LCD_ENPIN_SET_LOW(); // en pin low
	//timer_busysleep(&tlcd, US_2_TICKS(100));
}

void lcd_pulse_pause(void)
{
	// wait the rest of 100us
	timer_busysleep(&tlcd, US_2_TICKS(100));
}

void init_lcd(void)
{
	// data lines (D4 - D7) .. default L
	enable_port_clock(PORTC);
	GPIO_PORTC_DEN_R |= 0xF0;
	GPIO_PORTC_DIR_R |= 0xF0;
	GPIO_PORTC_DATA_R &= ~0xF0;

	// data lines (D3 - D2) .. default L
	enable_port_clock(PORTB);
	GPIO_PORTB_DEN_R |= 0xC0;
	GPIO_PORTB_DIR_R |= 0xC0;
	GPIO_PORTB_DATA_R &= ~0xC0;

	// data lines (D1 - D0) .. default L
	enable_port_clock(PORTD);
    GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
    GPIO_PORTD_CR_R |= 0x80; /* Allow D1 to be changed */
    GPIO_PORTD_LOCK_R = 0; /* Lock CR again */
	GPIO_PORTD_DEN_R |= 0xC0;
	GPIO_PORTD_DIR_R |= 0xC0;
	GPIO_PORTD_DATA_R &= ~0xC0;

	// RS, RW and EN pin .. default L
	enable_port_clock(PORTA);
	GPIO_PORTA_DEN_R |= 0x1C;
	GPIO_PORTA_DIR_R |= 0x1C;
	GPIO_PORTA_DATA_R &= ~0x1C;

	// wait 50ms
	int waitcnt=0;
	while (waitcnt<50) {if (SYSTICK_COUNT()) waitcnt++;}

	// send command 0x0011NFxx (function set)
	lcd_pulse_data(0x38,1);

	// wait more than 4.5ms
	timer_busysleep(&tlcd,US_2_TICKS(4500));

	// send command 0x0011NFxx (function set) again
	lcd_pulse_data(0x38,1);

	// wait more than 150us
	timer_busysleep(&tlcd,US_2_TICKS(150));

	// send command 0x0011NFxx (function set) again again
	lcd_pulse_data(0x38,1);
	lcd_pulse_pause();

	// send command 0x0011NFxx (function set) again again again
	lcd_pulse_data(0x38,1);
	lcd_pulse_pause();

	// send command 0x00001DCB (display ON/OFF)
	lcd_pulse_data(0x0C,1);
	// wait more than 2ms
	timer_busysleep(&tlcd,US_2_TICKS(2000));

	// send command 0x00000001 (clear display)
	lcd_pulse_data(0x01,1);
	// wait more than 2ms
	timer_busysleep(&tlcd,US_2_TICKS(2000));

	// send command 0x000001I/DS (entry mode set)
	lcd_pulse_data(0x06,1);
	lcd_pulse_pause();
}

void lcd_clearscr_wait()
{
    // Clear display
    lcd_pulse_data(0x02,1);
	timer_busysleep(&tlcd,US_2_TICKS(2000));
}

void lcd_prints_wait(char *Text)
{
    char *c;
    c = Text;

    while(*c!='\0')
    {
    	lcd_pulse_data(*c++,0);
    	lcd_pulse_pause();
    }
}

void lcd_goto_wait(char Row, char Col)
{
    uint8_t address = 0;

    // construct address from (Row, Col) pair
    if (Row != 0) address|=0x40;
    address |= Col;

    lcd_pulse_data(0x80 | address,1);
    lcd_pulse_pause();
}

/***** MAIN *****/

void main(void)
{
	/*int lcd_status = 0;
	timer_t lcd_timer;*/

	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);
	init_timer();

	init_lcd();

	//lcd_clearscr_wait();

	lcd_goto_wait(0,0);
	lcd_prints_wait(" co je noveho? .\0");
	lcd_goto_wait(1,0);
	lcd_prints_wait("nic,jako obvykle\0");

	lcd_goto_wait(0,0);

	while(1)
	{
		if (BUTTON_PRESSED(BUTTON_ONE)) LED_ON(LED_RED)
		else LED_OFF(LED_RED);
		if (BUTTON_PRESSED(BUTTON_TWO)) LED_ON(LED_GREEN)
		else LED_OFF(LED_GREEN);

		if (SYSTICK_COUNT()) // 1ms timing
		{
			static int bluetick = 0;
			bluetick++;

			if (bluetick==990)
			{
				LED_ON(LED_BLUE);
				LCD_RSPIN_SET_HIGH();
				LCD_RWPIN_SET_HIGH();
				LCD_ENPIN_SET_HIGH();
			}
			else if (bluetick==1000)
			{
				static char str[2]={'a',0};
				lcd_prints_wait(str);
				str[0]++;
				bluetick=0;
				LED_OFF(LED_BLUE);
			}
		}
	}
}
