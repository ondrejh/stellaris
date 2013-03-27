/*
 * lcd.h
 *
 *  Created on: 27.3.2013
 *      Author: ohejda
 */

#ifndef LCD_H_
#define LCD_H_

// module global includes
#include <inttypes.h>
#include "lm4f120h5qr.h"

void init_lcd(void);
void lcd_refresh(void);
void lcd_prints_buf(uint8_t pos,char *Text);
void lcd_putch_buf(uint8_t pos,char chr);

#endif /* LCD_H_ */
