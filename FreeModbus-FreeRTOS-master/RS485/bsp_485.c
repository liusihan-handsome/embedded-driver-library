	
//#include "led/bsp_led.h"    
#include "bsp_485.h" 
#include "bsp_TiMbase.h"
#include "shell.h"

#include "mb.h"
#include "mb_m.h"
#include "mbfunc.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"
#include <stdlib.h>

UART_HandleTypeDef huart2;

uint8_t RS485_RX_BUF[RS485_DATA_LEN];  	//接收缓冲,最大64个字节.
uint8_t RS485_RX_CNT = 0;   	            //接收到的数据长度	  

void RS485_Init(uint32_t ulBaudRate, uint8_t ucDataBits, uint8_t eParity)
{  
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
		
    GPIO_InitStruct.Pin = RS485_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RS485_EN_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    huart2.Instance = USART2;
    huart2.Init.BaudRate = ulBaudRate;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
		switch ( ucDataBits )
		{
			case 8:
				if (eParity == 0)
					huart2.Init.WordLength = UART_WORDLENGTH_8B;
				else
					huart2.Init.WordLength = UART_WORDLENGTH_9B;
				break;
			case 7:
				break;
			default:
				break;
		}
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
		 switch ( eParity )
		{
			case 0:
				huart2.Init.Parity = UART_PARITY_NONE;
				break;
			case 1:
				huart2.Init.Parity = UART_PARITY_ODD;
				break;
			case 2:
				huart2.Init.Parity = UART_PARITY_EVEN;
				break;
		}
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2); 

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    
    RS485_RX_MODE();
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
}

//void pvRS485_IRQHandler(void)
//{
//    uint8_t res = 0;
//    
//    if(0==USART_RS485Rx_STA)
//    {
//        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
//        HAL_TIM_Base_Start_IT(&htim3);
//    }
//    res = (uint8_t)huart2.Instance->DR;
//    USART_RS485RX_BUF[USART_RS485Rx_STA] = res;
//    USART_RS485Rx_STA++;
//    __HAL_TIM_SET_COUNTER(&htim3, 0);    //重设计数时间
//}

void RS485_Send_Data(uint8_t *Tx_Buf,uint8_t TxCount)
{
    RS485_TX_MODE();
    HAL_UART_Transmit(&huart2, Tx_Buf, TxCount, 0xffff);
    RS485_RX_MODE();
}


//uint8_t open_relay[] = {0x33, 0x01 , 0x14 , 0x00, 0x00, 0x00, 0x00, 0x48};
//uint8_t close_relay[] = {0x33, 0x01 , 0x13 , 0x00, 0x00, 0x00, 0x00, 0x47};
/*
uint8_t open_relay[] = {0XFE,0X05,0X00,0X00,0XFF,0X00,0X98,0X35};
uint8_t close_relay[] = {0XFE,0X05,0X00,0X00,0X00,0X00,0XD9,0XC5};


void openrelay(void)
{
	RS485_Send_Data( open_relay , 8);
}

void closerelay(void)
{
	RS485_Send_Data( close_relay , 8);
}
*/




//eMBMasterReqReadCoils(UCHAR ucSndAddr, USHORT usCoilAddr, USHORT usNCoils, LONG lTimeOut)
int modbusread(int argc, char **argv)
{
	if(argc != 4)
	{
		printf("\r\n Each data content is filled in decimal");
		printf("\r\n e:modbusread  SndAddr CoilAddr NCoils");
		printf("\r\n e:modbusread    1         0        0 \r\n");
		return 0;
	}
	UCHAR ucSndAddr = atoi(argv[1]);
	USHORT usCoilAddr = atoi(argv[2]);
	USHORT usNCoils = atoi(argv[3]);
	
	printf("\r\n ucSndAddr(%d) usCoilAddr(%d) usNCoils(%d) \r\n",ucSndAddr, usCoilAddr, usNCoils);
	eMBMasterReqReadCoils(ucSndAddr, usCoilAddr, usNCoils, 100);
}

int modbuswrite(int argc, char **argv)
{
	if(argc != 4)
	{
		printf("\r\n Each data content is filled in decimal");
		printf("\r\n e:modbuswrite  SndAddr CoilAddr  CoilData");
		printf("\r\n e:modbuswrite    1         0        0 \r\n");
		return 0;
	}
	UCHAR ucSndAddr = atoi(argv[1]);
	USHORT usCoilAddr = atoi(argv[2]);
	USHORT usCoilData = atoi(argv[3]);
	
	printf("\r\n ucSndAddr(%d) usCoilAddr(%d) usCoilData(%d) \r\n",ucSndAddr, usCoilAddr, usCoilData);
	eMBMasterReqWriteCoil(ucSndAddr, usCoilAddr, usCoilData, 100);
}
/*
int modbureadtest()
{
	eMBMasterReqReadCoils(0x01, 0x0000, 0x0010, 100);
}

int modbuwritetest()
{
	eMBMasterReqWriteCoil(0x01, 0x0001, 0xff00, 100);
}
*/
//ZNS_CMD_EXPORT(openrelay, open relay);
//ZNS_CMD_EXPORT(closerelay, close relay);
ZNS_CMD_EXPORT(modbusread, modbusread);
ZNS_CMD_EXPORT(modbuswrite, modbuswrite);
//ZNS_CMD_EXPORT(modbureadtest, modbu read test);
//ZNS_CMD_EXPORT(modbuwritetest, modbu write test);
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    
//}
