
#ifndef __BSP_485_H
#define __BSP_485_H	 

#ifdef __cplusplus
extern "C" {
#endif

//#include "delay.h"
#include <stdint.h>
#include "stm32f1xx_hal.h"

#define USE_RS485       1 
#define RS485_DATA_LEN  128
#define RS485_EN_Pin              GPIO_PIN_5
#define RS485_EN_GPIO_Port        GPIOC
#define RS485_RX_MODE()           HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET)
#define RS485_TX_MODE()           HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_SET)

extern __IO uint16_t USART_RS485Rx_STA;
extern __IO uint8_t USART_RS485RX_BUF[256];

extern void RS485_Init(uint32_t ulBaudRate, uint8_t ucDataBits, uint8_t eParity);

extern UART_HandleTypeDef huart2;
#ifdef __cplusplus
}
#endif
		 				    
#endif



