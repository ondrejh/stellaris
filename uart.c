/*
 * uart.c uart module source file
 *
 *  Created on: 28.3.2013
 *      Author: ohejda
 */

#include "uart.h"
#include "utils.h"
#include "gpios.h"

/**
 * uart 0 initialization function
 * 115200Baud 8N1, fifo enabled
 */
void init_uart0 ( void )
{
	//unsigned int ra;
    //16000000 / (16 * 115200 ) = 1000000 / 115200 = 8.6805
    //0.6805... * 64 = 43.5   ~ 44
    //use 8 and 44
    /*ra=GET32(RCGCUART);
    ra|=1; //UART0
    PUT32(RCGCUART,ra);*/
    SYSCTL_RCGCUART_R |= 1; // UART0
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

    UART0_LCRH_R |= UART_LCRH_FEN; // enable fifo
}

/**
 * uart 0 putchar function
 * it just inserts char into tx buffer and return true
 * if buffer is full return false
 */
bool uart0_putchar(char c)
{
	if (!(UART0_FR_R & UART_FR_TXFF))
	{
		UART0_DR_R = c;
		return true;
	}
	return false;
}

/**
 * uart 0 getchar function
 * it tests if there is some char in the buffer
 * if any copies it to "c" and return true
 * if non returns false
 */
bool uart0_getchar(char *c)
{
	if(!(UART0_FR_R & UART_FR_RXFE))
	{
		*c = UART0_DR_R;
		return true;
	}
	return false;
}

/**
 * uart 1 initialization function
 * 19200Baud 8N1, fifo enabled
 */
void init_uart1 ( void )
{
    //16000000 / (16 * 19200 ) = 1000000 / 19200 = 52.0833
    //0.0833... * 64 = 5.333...   ~ 5
    //use 52 and 5

	SYSCTL_RCGCGPIO_R |= 0x02; // GPIOB
    GPIO_PORTB_AFSEL_R |= 0x03; // PB0, PB1
    GPIO_PORTB_DEN_R |= 0x03;
    GPIO_PORTB_PCTL_R |=0x11;

    SYSCTL_RCGCUART_R |= 0x02; // UART1

    // disable uart
    while(UART0_FR_R & UART_FR_BUSY) {};
    UART1_LCRH_R &= ~(UART_LCRH_FEN);
    UART1_CTL_R &= ~(UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE);

    UART1_CTL_R &= ~(UART_CTL_HSE); // disable high speed mode
    UART1_IBRD_R = 52;
    UART1_FBRD_R = 5;
    UART1_LCRH_R = UART_LCRH_WLEN_8;// 8bit  | UART_LCRH_FEN; enable fifo
    UART1_FR_R = 0; // clear flags

    // uart enable
    UART1_LCRH_R |= UART_LCRH_FEN; // enable fifo
    UART1_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);
}

/**
 * uart 1 putchar function
 * it just inserts char into tx buffer and return true
 * if buffer is full return false
 */
bool uart1_putchar(char c)
{
	if (!(UART1_FR_R & UART_FR_TXFF))
	{
		UART1_DR_R = c;
		return true;
	}
	return false;
}

/**
 * uart 1 getchar function
 * it tests if there is some char in the buffer
 * if any copies it to "c" and return true
 * if non returns false
 */
bool uart1_getchar(char *c)
{
	if(!(UART1_FR_R & UART_FR_RXFE))
	{
		*c = UART1_DR_R;
		return true;
	}
	return false;
}
