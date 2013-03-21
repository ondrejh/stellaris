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
#include <inttypes.h>

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

#define LCD_PULSE_DELAY (10*8)
#define LCD_INTERCMD_DELAY (100*8)
#define LCD_INIT_DELAY (100000*8)
#define LCD_STARTUP_DELAY (50000*8)

void lcd_set_dataport(uint8_t data)
{
	GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~0xF0) | (data<<4);
}

void lcd_pulse_en(void)
{
    //
    // pull EN bit low
    //
    /*LCM_OUT &= ~LCM_PIN_EN;
    __delay_cycles(LCM_PULSE_DELAY);*/
    LCD_ENPIN_SET_LOW();
    busy_sleep(LCD_PULSE_DELAY);

    //
    // pull EN bit high
    //
    /*LCM_OUT |= LCM_PIN_EN;
    __delay_cycles(LCM_PULSE_DELAY);*/
    LCD_ENPIN_SET_HIGH();
    busy_sleep(LCD_PULSE_DELAY);

    //
    // pull EN bit low again
    //
    /*LCM_OUT &= (~LCM_PIN_EN);
    __delay_cycles(LCM_PULSE_DELAY);*/
    LCD_ENPIN_SET_LOW();
    busy_sleep(LCD_PULSE_DELAY);
}

void lcd_send_command(uint8_t ByteToSend)
{
    //
    // clear out all pins
    //
    //LCM_OUT &= (~LCM_PIN_MASK);
	//LCD_RESET_ALL_PINS();

    //
    // set High Nibble (HN) -
    // usefulness of the identity mapping
    // apparent here. We can set the
    // // DB7 - DB4 just by setting P1.7 - P1.4
    // DB7 - DB4 just by setting P2.3 - P2.0
    // using a simple assignment
    //
    //LCM_OUT |= (ByteToSend & 0xF0);
    //LCM_OUT |= ((ByteToSend & 0xF0) >> 4);
	lcd_set_dataport(ByteToSend>>4);

    //LCM_OUT &= ~LCM_PIN_RS;
    LCD_RSPIN_SET_LOW();
    LCD_RWPIN_SET_LOW();
    //
    // we've set up the input voltages to the LCM.
    // Now tell it to read them.
    //
    lcd_pulse_en();

    //
    // set Low Nibble (LN) -
    // usefulness of the identity mapping
    // apparent here. We can set the
    // // DB7 - DB4 just by setting P1.7 - P1.4
    // DB7 - DB4 just by setting P2.3 - P2.0
    // using a simple assignment
    //
    //LCM_OUT &= (~LCM_PIN_MASK);
    //LCD_RESET_ALL_PINS();
    //LCM_OUT |= ((ByteToSend & 0x0F) << 4);
    lcd_set_dataport(ByteToSend & 0x0F);

    //LCM_OUT &= ~LCM_PIN_RS;
    LCD_RSPIN_SET_LOW();
    LCD_RWPIN_SET_LOW();

    //
    // we've set up the input voltages to the LCM.
    // Now tell it to read them.
    //
    //PulseLcm();
    lcd_pulse_en();

    //busy_sleep(LCD_INTERCMD_DELAY);
}

void lcd_send_data(uint8_t ByteToSend)
{
    //
    // clear out all pins
    //
    //LCM_OUT &= (~LCM_PIN_MASK);
	//LCD_RESET_ALL_PINS();

    //
    // set High Nibble (HN) -
    // usefulness of the identity mapping
    // apparent here. We can set the
    // // DB7 - DB4 just by setting P1.7 - P1.4
    // DB7 - DB4 just by setting P2.3 - P2.0
    // using a simple assignment
    //
    //LCM_OUT |= (ByteToSend & 0xF0);
    //LCM_OUT |= ((ByteToSend & 0xF0) >> 4);
	lcd_set_dataport(ByteToSend>>4);

    LCD_RSPIN_SET_HIGH();
    LCD_RWPIN_SET_LOW();
    //busy_sleep(LCD_PULSE_DELAY);
    //
    // we've set up the input voltages to the LCM.
    // Now tell it to read them.
    //
    lcd_pulse_en();

    //
    // set Low Nibble (LN) -
    // usefulness of the identity mapping
    // apparent here. We can set the
    // // DB7 - DB4 just by setting P1.7 - P1.4
    // DB7 - DB4 just by setting P2.3 - P2.0
    // using a simple assignment
    //
    //LCM_OUT &= (~LCM_PIN_MASK);
    //LCD_RESET_ALL_PINS();
    //LCM_OUT |= ((ByteToSend & 0x0F) << 4);
    lcd_set_dataport(ByteToSend & 0x0F);

    LCD_RSPIN_SET_HIGH();
    LCD_RWPIN_SET_LOW();
    //busy_sleep(LCD_PULSE_DELAY);

    //
    // we've set up the input voltages to the LCM.
    // Now tell it to read them.
    //
    //PulseLcm();
    lcd_pulse_en();

    //busy_sleep(LCD_INTERCMD_DELAY);
}

void lcd_clearscr()
{
    //
    // Clear display, return home
    //
    lcd_send_command(0x01);
    lcd_send_command(0x02);
}

void lcd_goto(char Row, char Col)
{
    char address;

    //
    // construct address from (Row, Col) pair
    //

    if (Row == 0)
    {
        address = 0;
    }
    else
    {
        address = 0x40;
    }

    address |= Col;
    lcd_send_command(0x80 | address);
}

void lcd_prints(char *Text)
{
    char *c;
    c = Text;

    while(*c!='\0') lcd_send_data(*c++);

    /*while ((c != 0) && (*c != 0))
    {
        lcd_send_data(*c);
        c++;
    }*/
}

void init_lcd(void)
{
	// data lines (D4 - D7) .. default L
	enable_port_clock(PORTC);
	GPIO_PORTC_DEN_R |= 0xF0;
	GPIO_PORTC_DIR_R |= 0xF0;
	GPIO_PORTC_DATA_R &= ~0xF0;

	// RS, RW and EN pin .. default L
	enable_port_clock(PORTA);
	GPIO_PORTA_DEN_R |= 0x1C;
	GPIO_PORTA_DIR_R |= 0x1C;
	GPIO_PORTA_DATA_R &= ~0x1C;

    lcd_pulse_en();
    //
    // wait for the LCM to warm up and reach
    // active regions. Remember MSPs can power
    // up much faster than the LCM.
    //
    busy_sleep(LCD_INIT_DELAY);

    //
    // initialize the LCM module
    //
    // 1. Set 4-bit input
    lcd_set_dataport(0x02);
    lcd_pulse_en();
    busy_sleep(LCD_INIT_DELAY);

    //
    // set 4-bit input - second time.
    // (as reqd by the spec.)
    //
    lcd_send_command(0x28);

    //
    // 2. Display on, cursor off, blink off
    //
    lcd_send_command(0x0C);

    //
    // 3. Cursor move auto-increment
    //
    lcd_send_command(0x06);

    // clear display and wait
    lcd_clearscr();
    busy_sleep(LCD_STARTUP_DELAY);
}


/***** MAIN *****/

void main(void)
{
	init_gpios();
	init_systick(SYSTICK_1MS_64MHZ);

	init_lcd();

	lcd_goto(0,0);
	lcd_prints("ahoj");
	lcd_goto(1,1);
	lcd_prints("vole");

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

			if (bluetick==990)
			{
				LED_ON(LED_BLUE);
				LCD_RSPIN_SET_HIGH();
				LCD_RWPIN_SET_HIGH();
				LCD_ENPIN_SET_HIGH();
				//lcd_clearscr();
				lcd_goto(1,10);
			}
			else if (bluetick==1000)
			{
				bluetick=0;
				LED_OFF(LED_BLUE);
				static int cnt = 0;
				char str[16];
				str[0]='0'+(cnt%100/10);
				str[1]='0'+(cnt%10);
				str[2]='\0';
				lcd_prints(str);
				cnt++;
			}

		}
	}
}
