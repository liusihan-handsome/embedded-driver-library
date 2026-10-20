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
 * File: $Id: portevent_m.c v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#include "port.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
// Event bit mask for xMBMasterPortEventGet()
#define MB_EVENT_POLL_MASK (EventBits_t)(EV_MASTER_READY |          \
                                         EV_MASTER_FRAME_RECEIVED | \
                                         EV_MASTER_EXECUTE |        \
                                         EV_MASTER_FRAME_SENT |     \
                                         EV_MASTER_ERROR_PROCESS)


// Event bit mask for eMBMasterWaitRequestFinish()
#define MB_EVENT_REQ_MASK (EventBits_t)(EV_MASTER_PROCESS_SUCCESS |       \
                                        EV_MASTER_ERROR_RESPOND_TIMEOUT | \
                                        EV_MASTER_ERROR_RECEIVE_DATA |    \
                                        EV_MASTER_ERROR_EXECUTE_FUNCTION)

/* ----------------------- Variables ----------------------------------------*/
static SemaphoreHandle_t xSemaphorMasterHdl;
static EventGroupHandle_t xEventGroupMasterHdl;
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortEventInit(void)
{
    xEventGroupMasterHdl = xEventGroupCreate();
    return TRUE;
}

BOOL xMBMasterPortEventPost(eMBMasterEventType eEvent)
{
    BOOL bStatus = FALSE;
    eMBMasterEventType eTempEvent = eEvent;

    if( bMBMasterTimerIsWithinException() == TRUE )
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        BaseType_t xResult = xEventGroupSetBitsFromISR( xEventGroupMasterHdl,
                             (EventBits_t) eTempEvent,
                             &xHigherPriorityTaskWoken );
        // Was the message posted successfully?
        if( xResult == pdPASS ) {
            // If xHigherPriorityTaskWoken is now set to pdTRUE
            // then a context switch should be requested.
            //if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            bStatus = TRUE;
        } else {
            bStatus = FALSE;
        }
    }
    else
    {
        // Set event bits if the function is called from task
        // The return result is not checked here because
        // It might be that event bit was cleared automatically as a
        // task that was waiting for the bit was removed from the Blocked state.
        (void) xEventGroupSetBits( xEventGroupMasterHdl, (EventBits_t)eTempEvent );
        bStatus = TRUE;
    }
    return bStatus;
}

BOOL xMBMasterPortEventGet(eMBMasterEventType *eEvent)
{
    EventBits_t uxBits;
    BOOL xEventHappened = FALSE;
    uxBits = xEventGroupWaitBits(xEventGroupMasterHdl, // The event group being tested.
                                 MB_EVENT_POLL_MASK,   // The bits within the event group to wait for.
                                 pdTRUE,               // Masked bits should be cleared before returning.
                                 pdFALSE,              // Don't wait for both bits, either bit will do.
                                 portMAX_DELAY);       // Wait forever for either bit to be set.

    // Check if poll event is correct
    if (uxBits & MB_EVENT_POLL_MASK)
    {
        *eEvent = (eMBMasterEventType)(uxBits & MB_EVENT_POLL_MASK);
        xEventHappened = TRUE;
    }
    else
    {
        *eEvent = (eMBMasterEventType)uxBits;
        xEventHappened = FALSE;
    }
    return xEventHappened;
}
/**
 * This function is initialize the OS resource for modbus master.
 * Note:The resource is define by OS.If you not use OS this function can be empty.
 *
 */
void vMBMasterOsResInit(void)
{
    xSemaphorMasterHdl = xSemaphoreCreateBinary();
}

/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
BOOL xMBMasterRunResTake(LONG lTimeOut)
{
    /*If waiting time is -1 .It will wait forever */
    xSemaphoreTake(xSemaphorMasterHdl, lTimeOut);
    return TRUE;
}

/**
 * This function is release Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be empty.
 *
 */
void vMBMasterRunResRelease(void)
{
    /* release resource */
    xSemaphoreGive(xSemaphorMasterHdl);
}

/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                    USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_RESPOND_TIMEOUT);

    /* You can add your code under here. */
}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                 USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_RECEIVE_DATA);

    /* You can add your code under here. */
}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                     USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_EXECUTE_FUNCTION);

    /* You can add your code under here. */
}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestSuccess(void)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_PROCESS_SUCCESS);

    /* You can add your code under here. */
}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish(void)
{
    eMBMasterReqErrCode eErrStatus = MB_MRE_NO_ERR;
    eMBMasterEventType xRecvedEvent;

    EventBits_t uxBits = xEventGroupWaitBits(xEventGroupMasterHdl, // The event group being tested.
                         MB_EVENT_REQ_MASK,    // The bits within the event group to wait for.
                         pdTRUE,               // Masked bits should be cleared before returning.
                         pdFALSE,              // Don't wait for both bits, either bit will do.
                         1000);                // Wait forever for either bit to be set. //portMAX_DELAY
    xRecvedEvent = (eMBMasterEventType)(uxBits);
    if (xRecvedEvent)
    {
        if (!(xRecvedEvent & MB_EVENT_REQ_MASK))
        {
            // if we wait for certain event bits but get from poll subset
        }
        if (xRecvedEvent & EV_MASTER_PROCESS_SUCCESS)
        {
            eErrStatus = MB_MRE_NO_ERR;
        }
        else if (xRecvedEvent & EV_MASTER_ERROR_RESPOND_TIMEOUT)
        {
            eErrStatus = MB_MRE_TIMEDOUT;
        }
        else if (xRecvedEvent & EV_MASTER_ERROR_RECEIVE_DATA)
        {
            eErrStatus = MB_MRE_REV_DATA;
        }
        else if (xRecvedEvent & EV_MASTER_ERROR_EXECUTE_FUNCTION)
        {
            eErrStatus = MB_MRE_EXE_FUN;
        }
    }

    return eErrStatus;
}

#endif
