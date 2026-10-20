
#ifndef __BSP_TIMEBASE_H
#define __BSP_TIMEBASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define  MODBUS_TIM              TIM3 

extern TIM_HandleTypeDef htim3;

extern void MX_TIM3_Init(uint16_t arr,uint16_t psc);

#ifdef __cplusplus
extern "C" {
#endif

#endif	


