/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */
#include "FreeRTOS.h"
#include "task.h"

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbframe.h"
#include "event_groups.h"

//#include "led/bsp_led.h"    
#include "bsp_485.h" 
#include "shell.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/
/* serial transmit event */
#ifndef EVENT_SERIAL_TRANS_START
#define EVENT_SERIAL_TRANS_START    (1<<0)
#endif


#define MB_SERIAL_TASK_STACK_SIZE 512
#define MB_SERIAL_TASK_PRIO 10


/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    /**
     * set 485 mode receive and transmit control IO
     * @note MODBUS_MASTER_RT_CONTROL_PIN_INDEX need be defined by user
     */
    RS485_Init(ulBaudRate, (UCHAR)ucDataBits, (eMBParity)eParity); //RS485初始化
    return TRUE;
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	/* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    ENTER_CRITICAL_SECTION();
    if(xRxEnable==TRUE)//STM32串口 接收中断使能
    {
         __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);     //使能接收和接收中断
#if USE_RS485==1
        RS485_RX_MODE();
#endif
    }
    else if(xRxEnable == FALSE)
    {
        __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);   //禁止接收和接收中断
#if USE_RS485==1
        RS485_TX_MODE();
#endif
    }

    if(xTxEnable == TRUE)//STM32串口 发送中断使能
    {
        __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE); //使能发送完成中断
    }
    else if(xTxEnable == FALSE)
    {
        __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);//禁止发送完成中断
    }
    EXIT_CRITICAL_SECTION( );   
}


BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
#if USE_RS485==1
   RS485_TX_MODE();
#endif

    HAL_UART_Transmit(&huart2, (uint8_t *)&ucByte, 1, 10);  //串口发送1Byte数据
    return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR *pucByte)
{
    *pucByte = (uint8_t)(huart2.Instance->DR & (uint8_t)0x00FF); //串口接收1Byte数据
    return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.

void prvvUARTTxReadyISR(void)
{
    pxMBMasterFrameCBTransmitterEmpty();
}
 */

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.

void prvvUARTRxISR(void)
{
    pxMBMasterFrameCBByteReceived();
} 
*/
 
void vMasterUSARTHandler(void)
{
    BOOL bTaskWoken = FALSE;
		
    uint32_t tmp_flag = __HAL_UART_GET_FLAG(	&huart2, UART_FLAG_RXNE);
    uint32_t tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE);
    if ((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        bTaskWoken = pxMBMasterFrameCBByteReceived();
        __HAL_UART_CLEAR_FLAG(&huart2, UART_IT_RXNE);
    }
    if((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) != RESET)&&
        (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TXE) != RESET))
    {
        bTaskWoken = pxMBMasterFrameCBTransmitterEmpty();
    }
	
    portEND_SWITCHING_ISR(bTaskWoken ? pdTRUE : pdFALSE);
}

void USART2_IRQHandler(void)
{
	
	  HAL_UART_IRQHandler(&huart2);
	
    vMasterUSARTHandler();		
}

#endif
