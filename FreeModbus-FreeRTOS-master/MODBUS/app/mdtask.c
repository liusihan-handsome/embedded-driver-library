#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "mb.h"
#include "mbport.h"

#define REG_INPUT_START 4000
#define REG_INPUT_NREGS 8

#define REG_HOLDING_START (1)
#define REG_HOLDING_NREGS (8)

/* ----------------------- Static variables ---------------------------------*/

static USHORT usRegHoldingStart = REG_HOLDING_START;
static USHORT usRegHoldingBuf[REG_HOLDING_NREGS];

static USHORT usRegInputStart = REG_INPUT_START;
static USHORT usRegInputBuf[REG_INPUT_NREGS];

//************************ Modbus从机轮训线程***************************
void ModbusRTUTask(void const *argument)
{
    /* ABCDEF */
    usRegInputBuf[0] = 11;
    usRegInputBuf[1] = 22;
    usRegInputBuf[2] = 33;
    usRegInputBuf[3] = 44;
    usRegInputBuf[4] = 55;
    usRegInputBuf[5] = 66;
    usRegInputBuf[6] = 77;
    usRegInputBuf[7] = 88;
    usRegHoldingBuf[0] = 88;
    usRegHoldingBuf[1] = 77;
    //初始化协议栈
    eMBErrorCode eStatus = eMBInit(MB_RTU, 1, 3, 9600, MB_PAR_NONE);
    //启用协议栈
    eStatus = eMBEnable();

    while (1)
    { //启用协议栈的主循环池
        eMBPoll();
    }
}
//处理输入寄存器（D 只读）
eMBErrorCode
eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;

    if ((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        iRegIndex = (int)(usAddress - usRegInputStart);
        while (usNRegs > 0)
        {
            *pucRegBuffer++ =
                (unsigned char)(usRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ =
                (unsigned char)(usRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }

        //HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    }
    else
    {
        //HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
        eStatus = MB_ENOREG;
    }

    return eStatus;
}
//处理保持寄存器（D 读写）
eMBErrorCode
eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;

    if ((usAddress >= REG_HOLDING_START) && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = (int)(usAddress - usRegHoldingStart);
        switch (eMode)
        {
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (unsigned char)(usRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (unsigned char)(usRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
//处理输入继电器（X）
eMBErrorCode
eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils,
              eMBRegisterMode eMode)
{
    return MB_ENOREG;
}
//处理输出继电器（Y、M）
eMBErrorCode
eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
    return MB_ENOREG;
}
