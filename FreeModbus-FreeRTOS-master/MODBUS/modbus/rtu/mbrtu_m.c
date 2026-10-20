/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2013 China Beijing Armink <armink.ztl@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbrtu_m.c,v 1.60 2013/08/17 11:42:56 Armink Add Master Functions $
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/

#include "mb_m.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"
#include "elog.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_RTU_SER_PDU_SIZE_MIN 4 /*!< Minimum size of a Modbus RTU frame. */

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    STATE_M_RX_INIT,  /*!< Receiver is in initial state. */
    STATE_M_RX_IDLE,  /*!< Receiver is in idle state. */
    STATE_M_RX_RCV,   /*!< Frame is beeing received. */
    STATE_M_RX_ERROR, /*!< If the frame is invalid. */
} eMBMasterRcvState;

typedef enum
{
    STATE_M_TX_IDLE, /*!< Transmitter is in idle state. */
    STATE_M_TX_XMIT, /*!< Transmitter is in transfer state. */
    STATE_M_TX_XFWR, /*!< Transmitter is in transfer finish and wait receive state. */
} eMBMasterSndState;

#if MB_MASTER_RTU_ENABLED > 0
/*------------------------ Shared variables ---------------------------------*/


/* ----------------------- Static variables ---------------------------------*/
static volatile eMBMasterSndState eSndState;
static volatile eMBMasterRcvState eRcvState;

static volatile UCHAR ucMasterRTUSndBuf[MB_PDU_SIZE_MAX];
static volatile UCHAR ucMasterRTURcvBuf[MB_SER_PDU_SIZE_MAX];
static volatile USHORT usMasterSendPDULength;

static volatile UCHAR *pucMasterSndBufferCur;
static volatile USHORT usMasterSndBufferCount;
static volatile USHORT usMasterRcvBufferPos;
static volatile BOOL xFrameIsBroadcast = FALSE;

static volatile eMBMasterTimerMode eMasterCurTimerMode;

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBMasterRTUInit(UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity)
{
    eMBErrorCode eStatus = MB_ENOERR;
    ULONG usTimerT35_50us;

    ENTER_CRITICAL_SECTION();

    /* Modbus RTU uses 8 Databits. */
	//初始化串口 入参顺序： 端口号，波特率，数据为，校验位
    if (xMBMasterPortSerialInit(ucPort, ulBaudRate, 8, eParity) != TRUE)
    {
        eStatus = MB_EPORTERR;
    }
    else
    {//如果初始化串口成功则需要初始化定时器
        /* If baudrate > 19200 then we should use the fixed timer values
         * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
         */
		//根据协议，大于19200的波特率T3.5采用固定时间，否则T3.5根据波特率计算
        if (ulBaudRate > 19200)
        {
            usTimerT35_50us = 35; /* 1800us. */
        }
        else
        {
            /* The timer reload value for a character is given by:
             *
             * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
             *             = 11 * Ticks_per_1s / Baudrate
             *             = 220000 / Baudrate
             * The reload for t3.5 is 1.5 times this value and similary
             * for t3.5.
             */
            usTimerT35_50us = (7UL * 220000UL) / (2UL * ulBaudRate);
        }
		//初始化定时器 usTimerT35_50us为计算后的定时器装在值
        if (xMBMasterPortTimersInit((USHORT)usTimerT35_50us) != TRUE)
        {
            eStatus = MB_EPORTERR;
        }
    }
    EXIT_CRITICAL_SECTION();

    return eStatus;
}

/*函数功能 
*1:设置接收状态机eRcvState为STATE_RX_INIT； 
*2:使能串口接收,禁止串口发送,作为从机,等待主机传送的数据; 
*3:开启定时器，3.5T时间后定时器发生第一次中断,此时eRcvState为STATE_RX_INIT,上报初始化完成事件,然后设置eRcvState为空闲STATE_RX_IDLE; 
*4:每次进入3.5T定时器中断,定时器被禁止，等待串口有字节接收后，才使能定时器; 
*/
void eMBMasterRTUStart(void)
{//启动RTU主站
    ENTER_CRITICAL_SECTION();
    /* Initially the receiver is in the state STATE_M_RX_INIT. we start
     * the timer and if no character is received within t3.5 we change
     * to STATE_M_RX_IDLE. This makes sure that we delay startup of the
     * modbus protocol stack until the bus is free.
     */
    eRcvState = STATE_M_RX_IDLE; //STATE_M_RX_INIT (We start processing immediately)
    vMBMasterPortSerialEnable(TRUE, FALSE);
    vMBMasterPortTimersT35Enable();

    EXIT_CRITICAL_SECTION();
}

void eMBMasterRTUStop(void)
{//停止RTU主站
    ENTER_CRITICAL_SECTION();
    vMBMasterPortSerialEnable(FALSE, FALSE);
    vMBMasterPortTimersDisable();
    EXIT_CRITICAL_SECTION();
}

/*eMBPoll函数轮询到EV_FRAME_RECEIVED事件时,调用peMBFrameReceiveCur()，此函数是用户为函数指针peMBFrameReceiveCur()的赋值 
*此函数完成的功能：从一帧数据报文中，取得modbus从机地址给pucRcvAddress，PDU报文的长度给pusLength，PDU报文的首地址给pucFrame，函数 
*形参全部为地址传递*/
eMBErrorCode
eMBMasterRTUReceive(UCHAR *pucRcvAddress, UCHAR **pucFrame, USHORT *pusLength)
{
    eMBErrorCode eStatus = MB_ENOERR;
	
    ENTER_CRITICAL_SECTION();
    assert_param(usMasterRcvBufferPos < MB_SER_PDU_SIZE_MAX);/*断言宏，判断接收到的字节数<256，如果>256，终止程序*/

    /* Length and CRC check */
    if ((usMasterRcvBufferPos >= MB_SER_PDU_SIZE_MIN) && (usMBCRC16((UCHAR *)ucMasterRTURcvBuf, usMasterRcvBufferPos) == 0))
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        *pucRcvAddress = ucMasterRTURcvBuf[MB_SER_PDU_ADDR_OFF];//取接收到的第一个字节，modbus从机地址

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = (USHORT)(usMasterRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame = (UCHAR *)&ucMasterRTURcvBuf[MB_SER_PDU_PDU_OFF];
    }
    else
    {
        eStatus = MB_EIO;
    }

    EXIT_CRITICAL_SECTION();
    return eStatus;
}

/*函数功能 
*1:对响应报文PDU前面加上从机地址; 
*2:对响应报文PDU后加上CRC校; 
*3:使能发送，启动传输; 
*/
eMBErrorCode
eMBMasterRTUSend(UCHAR ucSlaveAddress, const UCHAR *pucFrame, USHORT usLength)
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT usCRC16;

    if (ucSlaveAddress > MB_MASTER_TOTAL_SLAVE_NUM)
        return MB_EINVAL;

    ENTER_CRITICAL_SECTION();

    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if (eRcvState == STATE_M_RX_IDLE)
    {
        /* First byte before the Modbus-PDU is the slave address. */
        /*在协议数据单元前加从机地址*/
        pucMasterSndBufferCur = (UCHAR *)pucFrame - 1;
        usMasterSndBufferCount = 1;

        /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
        pucMasterSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
        usMasterSndBufferCount += usLength;

        /* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
        usCRC16 = usMBCRC16((UCHAR *)pucMasterSndBufferCur, usMasterSndBufferCount);
        ucMasterRTUSndBuf[usMasterSndBufferCount++] = (UCHAR)(usCRC16 & 0xFF);
        ucMasterRTUSndBuf[usMasterSndBufferCount++] = (UCHAR)(usCRC16 >> 8);

        /* Activate the transmitter. */
        eSndState = STATE_M_TX_XMIT;//发送状态
        // The place to enable RS485 driver
        vMBMasterPortSerialEnable(FALSE, TRUE);
    }
    else
    {
        eStatus = MB_EIO;
    }
    EXIT_CRITICAL_SECTION();
    return eStatus;
}

/*函数功能  
*1:将接收到的数据存入ucRTUBuf[]中;  
*2:usRcvBufferPos为全局变量，表示接收数据的个数;  
*3:每接收到一个字节的数据，3.5T定时器清0  
*/
BOOL xMBMasterRTUReceiveFSM(void)
{
    BOOL xStatus = FALSE;
    UCHAR ucByte;

    assert_param((eSndState == STATE_M_TX_IDLE) || (eSndState == STATE_M_TX_XFWR));

    /* Always read the character. */
    xStatus = xMBMasterPortSerialGetByte((CHAR *)&ucByte); /*从串口数据寄存器读取一个字节数据*/
	
    switch (eRcvState) //根据不同的状态转移
    {
        /* If we have received a character in the init state we have to
         * wait until the frame is finished.
         */
    case STATE_M_RX_INIT:
        vMBMasterPortTimersT35Enable();/*开启3.5T定时器*/
        break;

        /* In the error state we wait until all characters in the
         * damaged frame are transmitted.
         */
    case STATE_M_RX_ERROR:/*数据帧被损坏，重启定时器，不保存串口接收的数据*/
        vMBMasterPortTimersT35Enable();
        break;

        /* In the idle state we wait for a new character. If a character
         * is received the t1.5 and t3.5 timers are started and the
         * receiver is in the state STATE_RX_RECEIVCE and disable early
         * the timer of respond timeout .
         */
    case STATE_M_RX_IDLE: /*接收器空闲，开始接收，进入STATE_RX_RCV状态*/
        /* In time of respond timeout,the receiver receive a frame.
         * Disable timer of respond timeout and change the transmiter state to idle.
         */
        vMBMasterPortTimersDisable();
        eSndState = STATE_M_TX_IDLE;

        usMasterRcvBufferPos = 0;
        ucMasterRTURcvBuf[usMasterRcvBufferPos++] = ucByte;/*保存数据*/
        eRcvState = STATE_M_RX_RCV;

        /* Enable t3.5 timers. */
        vMBMasterPortTimersT35Enable();/*每收到一个字节，都重启3.5T定时器*/
        break;

        /* We are currently receiving a frame. Reset the timer after
         * every character received. If more than the maximum possible
         * number of bytes in a modbus frame is received the frame is
         * ignored.
         */
    case STATE_M_RX_RCV:
        if (usMasterRcvBufferPos < MB_SER_PDU_SIZE_MAX)
        {
            if (xStatus)
            {
                ucMasterRTURcvBuf[usMasterRcvBufferPos++] = ucByte; /*接收数据*/
            }
        }
        else
        {
            eRcvState = STATE_M_RX_ERROR;/*一帧报文的字节数大于最大PDU长度，忽略超出的数据*/
        }
        vMBMasterPortTimersT35Enable();/*每收到一个字节，都重启3.5T定时器*/
        break;
    }
    return xStatus;
}

BOOL xMBMasterRTUTransmitFSM(void)
{
    BOOL xNeedPoll = FALSE;

    assert_param(eRcvState == STATE_M_RX_IDLE);

    switch (eSndState)
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_M_TX_XFWR:
        xNeedPoll = FALSE;
        break;

    case STATE_M_TX_IDLE: /*发送器处于空闲状态，使能接收，禁止发送*/
        /* enable receiver/disable transmitter. */
        vMBMasterPortSerialEnable(TRUE, FALSE);
        break;

    case STATE_M_TX_XMIT: /*发送器处于发送状态,在从机发送函数eMBRTUSend中赋值STATE_TX_XMIT*/
        /* check if we are finished. */
        if (usMasterSndBufferCount != 0)
        { //发送数据
            xMBMasterPortSerialPutByte((CHAR)*pucMasterSndBufferCur);
            pucMasterSndBufferCur++; /* next byte in sendbuffer. */
            usMasterSndBufferCount--;
        }
        else
        { //传递任务，发送完成
            vMBMasterRequestSetType((ucMasterRTUSndBuf[MB_SER_PDU_ADDR_OFF] == MB_ADDRESS_BROADCAST) ? TRUE : FALSE);
            /* Disable transmitter. This prevents another transmit buffer
             * empty interrupt. */
            vMBMasterPortSerialEnable(TRUE, FALSE);/*使能接收，禁止发送*/
            eSndState = STATE_M_TX_XFWR;
            /* If the frame is broadcast ,master will enable timer of convert delay,
             * else master will enable timer of respond timeout. */
            if (xFrameIsBroadcast == TRUE)
            {
                vMBMasterPortTimersConvertDelayEnable();
            }
            else
            {
                vMBMasterPortTimersRespondTimeoutEnable();
            }
        }
        break;

    default:
        break;
    }

    return xNeedPoll;
}

/*函数功能 
*1:接受完成一帧数据后，接收状态机eRcvState为STATE_RX_RCV； 
*2:上报“接收到报文”事件(EV_FRAME_RECEIVED) 
*3:禁止3.5T定时器，设置接收状态机eRcvState状态为STATE_RX_IDLE空闲; 
*/
BOOL xMBMasterRTUTimerExpired(void)
{
    BOOL xNeedPoll = FALSE;

    switch (eRcvState)
    {
        /* Timer t35 expired. Startup phase is finished. */
    /*上报modbus协议栈的事件状态给poll函数,EV_READY:初始化完成事件*/
    case STATE_M_RX_INIT:
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_READY);
        break;

        /* A frame was received and t35 expired. Notify the listener that
		 * a new frame was received. */
    case STATE_M_RX_RCV:/*一帧数据接收完成*/
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_FRAME_RECEIVED);/*上报协议栈事件,接收到一帧完整的数据*/
        break;

        /* An error occured while receiving the frame. */
    case STATE_M_RX_ERROR:
        vMBMasterSetErrorType(EV_ERROR_RECEIVE_DATA);
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_ERROR_PROCESS);
        break;

        /* Function called in an illegal state. */
    default:
        assert_param(
            (eRcvState == STATE_M_RX_INIT) || (eRcvState == STATE_M_RX_RCV) ||
            (eRcvState == STATE_M_RX_ERROR) || (eRcvState == STATE_M_RX_IDLE));
        break;
    }
    eRcvState = STATE_M_RX_IDLE;

    switch (eSndState)
    {
        /* A frame was send finish and convert delay or respond timeout expired.
		 * If the frame is broadcast,The master will idle,and if the frame is not
		 * broadcast.Notify the listener process error.*/
    case STATE_M_TX_XFWR:
        if (xFrameIsBroadcast == FALSE)
        {
            vMBMasterSetErrorType(EV_ERROR_RESPOND_TIMEOUT);
            xNeedPoll = xMBMasterPortEventPost(EV_MASTER_ERROR_PROCESS);			
        }
        break;
        /* Function called in an illegal state. */
    default:
        assert_param(
		(eSndState == STATE_M_TX_XMIT) || (eSndState == STATE_M_TX_IDLE));
        break;
    }
    eSndState = STATE_M_TX_IDLE;

    vMBMasterPortTimersDisable();
    /* If timer mode is convert delay, the master event then turns EV_MASTER_EXECUTE status. */
    if (xMBMasterGetCurTimerMode() == MB_TMODE_CONVERT_DELAY)
    {
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_EXECUTE);
    }

    return xNeedPoll;
}


/* Get Modbus Master send PDU's buffer address pointer.*/
void vMBMasterGetPDUSndBuf(UCHAR **pucFrame)
{
    *pucFrame = (UCHAR *)&ucMasterRTUSndBuf[MB_SER_PDU_PDU_OFF];
}

/* Set Modbus Master send PDU's buffer length.*/
void vMBMasterSetPDUSndLength(USHORT SendPDULength)
{
    usMasterSendPDULength = SendPDULength;
}

/* Get Modbus Master send PDU's buffer length.*/
USHORT usMBMasterGetPDUSndLength(void)
{
    return usMasterSendPDULength;
}

/* Set Modbus Master current timer mode.*/
void vMBMasterSetCurTimerMode(eMBMasterTimerMode eMBTimerMode)
{
    eMasterCurTimerMode = eMBTimerMode;
}

/* Get Modbus Master current timer mode.*/
eMBMasterTimerMode  xMBMasterGetCurTimerMode(void)
{
    return eMasterCurTimerMode;
}

/* The master request is broadcast? */
BOOL xMBMasterRequestIsBroadcast(void)
{
    return xFrameIsBroadcast;
}

/* The master request is broadcast? */
void vMBMasterRequestSetType( BOOL xIsBroadcast ){
    xFrameIsBroadcast = xIsBroadcast;
}
#endif
