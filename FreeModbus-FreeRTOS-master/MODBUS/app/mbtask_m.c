
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"
#include "user_mb_app.h"

#include "bsp_485.h" 
#include "mbport.h"
#include <elog.h>
#if MB_MASTER_RTU_ENABLED == 1
#include "mbrtu.h"
#endif
#if MB_MASTER_ASCII_ENABLED == 1
#include "mbascii.h"
#endif
#if MB_MASTER_TCP_ENABLED == 1
#include "mbtcp.h"
#endif


eMBErrorCode eMBMasterRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
		printf("eMBMasterRegDiscreteCB buffer(%s) ,usAddress(%x) usNDiscrete(%d)" , pucRegBuffer, usAddress , usNDiscrete);
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex, iRegBitIndex, iNReg;
    UCHAR *pucDiscreteInputBuf;
    USHORT DISCRETE_INPUT_START;
    USHORT DISCRETE_INPUT_NDISCRETES;
    USHORT usDiscreteInputStart;
    iNReg = usNDiscrete / 8 + 1;

    pucDiscreteInputBuf = ucMDiscInBuf[ucMBMasterGetDestAddress() - 1];
    DISCRETE_INPUT_START = M_DISCRETE_INPUT_START;
    DISCRETE_INPUT_NDISCRETES = M_DISCRETE_INPUT_NDISCRETES;
    usDiscreteInputStart = usMDiscInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= DISCRETE_INPUT_START) && (usAddress + usNDiscrete <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES))
    {
        iRegIndex = (USHORT)(usAddress - usDiscreteInputStart) / 8;
        iRegBitIndex = (USHORT)(usAddress - usDiscreteInputStart) % 8;

        /* write current discrete values with new values from the protocol stack. */
        while (iNReg > 1)
        {
            xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex, 8, *pucRegBuffer++);
            iNReg--;
        }
        /* last discrete */
        usNDiscrete = usNDiscrete % 8;
        /* xMBUtilSetBits has bug when ucNBits is zero */
        if (usNDiscrete != 0)
        {
            xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex,
                           usNDiscrete, *pucRegBuffer++);
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode eMBMasterRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
	 printf("eMBMasterRegCoilsCB buffer(%s) ,usAddress(%x) usNCoils(%d) eMode(%d)" , pucRegBuffer, usAddress , usNCoils , eMode);
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex, iRegBitIndex, iNReg;
    UCHAR *pucCoilBuf;
    USHORT COIL_START;
    USHORT COIL_NCOILS;
    USHORT usCoilStart;
    iNReg = usNCoils / 8 + 1;

    pucCoilBuf = ucMCoilBuf[ucMBMasterGetDestAddress() - 1];
    COIL_START = M_COIL_START;
    COIL_NCOILS = M_COIL_NCOILS;
    usCoilStart = usMCoilStart;

    /* it already plus one in modbus function method. */
    usAddress--;
    if ((usAddress >= COIL_START) && (usAddress + usNCoils <= COIL_START + COIL_NCOILS))
    {
        iRegIndex = (USHORT)(usAddress - usCoilStart) / 8;
        iRegBitIndex = (USHORT)(usAddress - usCoilStart) % 8;

        while (iNReg > 1)
        {
            xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8, *pucRegBuffer++);
            iNReg--;
        }
        /* last coils */
        usNCoils = usNCoils % 8;
        /* xMBUtilSetBits has bug when ucNBits is zero */
        if (usNCoils != 0)
        {
            xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils, *pucRegBuffer++);
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode eMBMasterRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
	  printf("eMBMasterRegHoldingCB buffer(%s) ,usAddress(%x) usNRegs(%d) eMode(%d)" , pucRegBuffer, usAddress , usNRegs , eMode);
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex;
    USHORT *pusRegHoldingBuf;
    USHORT REG_HOLDING_START;
    USHORT REG_HOLDING_NREGS;
    USHORT usRegHoldStart;

    pusRegHoldingBuf = usMRegHoldBuf[ucMBMasterGetDestAddress() - 1];
    REG_HOLDING_START = M_REG_HOLDING_START;
    REG_HOLDING_NREGS = M_REG_HOLDING_NREGS;
    usRegHoldStart = usMRegHoldStart;
    /* if mode is read, the master will write the received date to buffer. */
    eMode = MB_REG_WRITE;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_HOLDING_START) && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR)(pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR)(pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;
        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode eMBMasterRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
		printf("eMBMasterRegHoldingCB buffer(%s) ,usAddress(%x) usNRegs(%d) " , pucRegBuffer, usAddress , usNRegs );
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex;
    USHORT *pusRegInputBuf;
    USHORT REG_INPUT_START;
    USHORT REG_INPUT_NREGS;
    USHORT usRegInStart;

    pusRegInputBuf = usMRegInBuf[ucMBMasterGetDestAddress() - 1];
    REG_INPUT_START = M_REG_INPUT_START;
    REG_INPUT_NREGS = M_REG_INPUT_NREGS;
    usRegInStart = usMRegInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        iRegIndex = usAddress - usRegInStart;
        while (usNRegs > 0)
        {
            pusRegInputBuf[iRegIndex] = *pucRegBuffer++ << 8;
            pusRegInputBuf[iRegIndex] |= *pucRegBuffer++;
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}
