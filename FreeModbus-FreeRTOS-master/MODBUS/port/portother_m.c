/*
 * FreeModbus Libary: ARM7/AT91SAM7X Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
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
 * File: $Id$
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/
void            vMBMasterPortSerialClose( void );
void            vMBMasterPortEventClose( void );
void            vMBMasterPortTimerClose( void );

/* ----------------------- Static variables ---------------------------------*/
static BOOL     bIsWithinException = FALSE;
static UCHAR ucPortMode = 0;

/* ----------------------- Start implementation -----------------------------*/

void
vMBMasterTimerSetWithinException( BOOL bInException )
{
    bIsWithinException = bInException;
}

BOOL bMBMasterTimerIsWithinException( void )
{
    return bIsWithinException;
}

void vMBMasterPortEnterCritical( void )
{
    taskENTER_CRITICAL(  );
}

void vMBMasterPortExitCritical( void )
{
    taskEXIT_CRITICAL(  );
}

UCHAR ucMBPortGetMode( void )
{
    return ucPortMode;
}

void vMBPortSetMode( UCHAR ucMode )
{
    ENTER_CRITICAL_SECTION();
    ucPortMode = ucMode;
    EXIT_CRITICAL_SECTION();
}


//void vMBMasterPortClose( void )
//{    
//    vMBMasterPortSerialClose(  );
//    vMBMasterPortTimerClose(  );
//    vMBMasterPortEventClose(  );
//}

