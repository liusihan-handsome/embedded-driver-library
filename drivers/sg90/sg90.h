#ifndef __SG90_H__
#define __SG90_H__

#include <stdint.h>
#include "stm32f1xx_hal.h"

void sg90_init(void);
void sg90_angle_set(uint16_t angle);

#endif
