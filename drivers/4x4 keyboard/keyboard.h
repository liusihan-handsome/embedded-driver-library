#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include "stm32f1xx_hal.h"

void keyboard_init(void);
uint8_t keyboard_get_value(void);

#endif
