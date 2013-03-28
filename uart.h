/*
 * uart.h uart module header file
 *
 *  Created on: 28.3.2013
 *      Author: ohejda
 */

#ifndef UART_H_
#define UART_H_

#include "lm4f120h5qr.h"
#include <stdbool.h>

// initialization
void init_uart0 ( void );

// putchar / getchar function
bool uart0_putchar(char c);
bool uart0_getchar(char *c);

#endif /* UART_H_ */
