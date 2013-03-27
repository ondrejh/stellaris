/*
 * lcd.c
 *
 *  Created on: 27.3.2013
 *      Author: ohejda
 */

#include "utils.h"
#include "lcd.h"
#include "gpios.h"

#define LCD_RSPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x04;}
#define LCD_RSPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x04;}
#define LCD_RWPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x08;}
#define LCD_RWPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x08;}
#define LCD_ENPIN_SET_LOW() {GPIO_PORTA_DATA_R&=~0x10;}
#define LCD_ENPIN_SET_HIGH() {GPIO_PORTA_DATA_R|=0x10;}

// internal lcd timer
timer_t tlcd;

// lcd buffer
char lcd_buffer[32] = 	{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '\
						,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

/**
 * function copying data word into the port pins D0..D7
 */
void lcd_set_dataport(uint8_t data)
{
	GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~0xF0) | (data&0xF0);
	GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & ~0xC0) | ((data&0x0C)<<4);
	GPIO_PORTD_DATA_R = (GPIO_PORTD_DATA_R & ~0xC0) | ((data&0x03)<<6);
}

/**
 * function pulsing command or data to lcd
 * its implemented without wating / the lcd_pulse_pause should be inserted between two calls
 */
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

/**
 * busysleep pause between two command/data writes
 */
void lcd_pulse_pause(void)
{
	// wait the rest of 100us
	timer_busysleep(&tlcd, US_2_TICKS(100));
}

/**
 * lcd initialization function (initialize port pins and go through lcd init sequence)
 */
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
	timer_busysleep(&tlcd,US_2_TICKS(50000));

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

/**
 * direct lcd clearscr function
 * implemented with waiting
 */
void lcd_clearscr_wait()
{
    // Clear display
    lcd_pulse_data(0x02,1);
	timer_busysleep(&tlcd,US_2_TICKS(2000));
}

/**
 * direct lcd prints function
 * implemented with waiting
 */
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

/**
 * goto function for lcd (set lcd cursor position)
 * implemented with waiting
 */
void lcd_goto_wait(char Row, char Col)
{
    uint8_t address = 0;

    // construct address from (Row, Col) pair
    if (Row != 0) address|=0x40;
    address |= Col;

    lcd_pulse_data(0x80 | address,1);
    lcd_pulse_pause();
}

/**
 * goto function for lcd (set lcd cursor position)
 * it's implemented without waiting - next goto can't be called sooner than in 100us
 */
void lcd_goto(uint8_t row, uint8_t col)
{
	uint8_t adr = 0;
	if (row!=0) adr |= 0x40;
	adr |= col;
	lcd_pulse_data(0x80 | adr,1);
}

/**
 * put char function for lcd
 * it's implemented without waiting - next putch can't be called sooner than in 100us
 */
void lcd_putch(char chr)
{
	lcd_pulse_data(chr,0);
}

/**
 * this function will refresh one character from lcd buffer in time
 * it should be called periodically but no more often than every 100us
 */
void lcd_refresh(void)
{
	static uint8_t row = 0;
	static uint8_t col = 0;

	if (col>15)
	{
		row ^= 1;
		col = 0;
		lcd_goto(row,col);
	}
	else
	{
		lcd_putch(lcd_buffer[row*16+col++]);
	}
}

/**
 * the function writing string into the lcd buffer
 */
void lcd_prints_buf(uint8_t pos,char *Text)
{
	uint8_t ptr = pos;
	while ((ptr<32)&&(*Text!='\0')) lcd_buffer[ptr++]=*Text++;
}

/**
 * the function writing one char into lcd buffer
 */
void lcd_putch_buf(uint8_t pos,char chr)
{
	if (pos<32) lcd_buffer[pos]=chr;
}
