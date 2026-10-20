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
 * File: $Id: porttimer_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#include "bsp_TiMbase.h"
#include "shell.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
/* ----------------------- Variables ----------------------------------------*/
static USHORT usTim1Timerout50us;

/* ----------------------- static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortTimersInit(USHORT usTimeOut50us)
{
    usTim1Timerout50us = usTimeOut50us;
    MX_TIM3_Init((uint16_t)usTim1Timerout50us,(uint16_t)(SystemCoreClock/20000) - 1); 
    return TRUE;
}

void vMBMasterPortTimersT35Enable()
{
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_T35);
    __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
		__HAL_TIM_SET_COUNTER(&htim3, 0);
		HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start(&htim3);
	
}

void vMBMasterPortTimersConvertDelayEnable()
{
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_CONVERT_DELAY);
   __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
	 __HAL_TIM_SET_COUNTER(&htim3, 0);
   __HAL_TIM_SET_AUTORELOAD(&htim3, MB_MASTER_DELAY_MS_CONVERT * 20 - 1);
//	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);   //这为啥是tim4 没懂， 写错了吗？
		HAL_TIM_Base_Start_IT(&htim3);
	
}

void vMBMasterPortTimersRespondTimeoutEnable()
{
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_RESPOND_TIMEOUT);
    __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
		__HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim3, MB_MASTER_TIMEOUT_MS_RESPOND*20 - 1);   //设定设定计数 即 MB_MASTER_DELAY_MS_CONVERT ms
		HAL_TIM_Base_Start_IT(&htim3);
}

void vMBMasterPortTimersDisable()
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    HAL_TIM_Base_Stop(&htim3);
}

void prvv_MASTERTIMERExpiredISR(void)
{
	BOOL bTaskWoken = FALSE;
   
    vMBMasterTimerSetWithinException(TRUE);
    
    if ( (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET) && (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET) )
    {
        __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
        pxMBMasterPortCBTimerExpired();
    }

    vMBMasterTimerSetWithinException(FALSE);

    portEND_SWITCHING_ISR(bTaskWoken ? pdTRUE : pdFALSE);

}

//定时器3中断服务程序
void TIM3_IRQHandler(void)  
{
	prvv_MASTERTIMERExpiredISR();
}

//ZNS_CMD_EXPORT(vMBMasterPortTimersT35Enable, vMBMasterPortTimersT35Enable test);
//ZNS_CMD_EXPORT(vMBMasterPortTimersConvertDelayEnable, vMBMasterPortTimersConvertDelayEnable test);
//ZNS_CMD_EXPORT(vMBMasterPortTimersRespondTimeoutEnable, vMBMasterPortTimersRespondTimeoutEnable test);
//ZNS_CMD_EXPORT(vMBMasterPortTimersDisable, vMBMasterPortTimersDisable test);

#endif
