#ifndef __LCD1602_H__
#define __LCD1602_H__

#include "stm32f1xx_hal.h"

void lcd1602_init(void);
void lcd1602_gpio_init(void);
void lcd1602_start(void);
void lcd1602_write_cmd(char cmd);
void lcd1602_write_data(char dataShow);
void lcd1602_show_char(void);

#endif
