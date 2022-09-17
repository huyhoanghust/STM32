/** 
Edit by modify: Ngoc Hang 
**/
#ifndef __i2clcd_h_
#define __i2clcd_h_

#include "stm32f1xx_hal.h"
#include "stdio.h"
#include <math.h>

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_clear_display (void);	//clear display lcd

void lcd_goto_XY (int row, int col); //set proper location on screen

void lcd_send_int_number(uint16_t number);

void lcd_send_float_number(float number, int afterpoint);

#endif