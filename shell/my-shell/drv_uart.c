#include "project.h"

loopbuf_t *buf;

void UART0_Handler(void)
{
	uint32_t chr;
	
	if(UART_INTStat(UART0, UART_IT_RX_THR))
	{
		while((UART0->FIFO & UART_FIFO_RXLVL_Msk) > 1)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
           loopbuf_write(buf,(const uint8_t*)(&chr),1);
			}
		}
	}
	else if(UART_INTStat(UART0, UART_IT_RX_TOUT))
	{
		while(UART_IsRXFIFOEmpty(UART0) == 0)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
                loopbuf_write(buf,(const uint8_t*)(&chr),1);
			}
		}
		
	}
	

}


void SerialInit(uint32_t Baudrate)
{
    buf=loopbuf_create(10);
    if(buf==NULL)
    {
        printf("loopbuf error\r\n");
    }
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);	//GPIOA.2配置为UART0输入引脚
	PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);	//GPIOA.3配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = Baudrate;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
    
	UART_initStruct.RXThreshold = 7;
	UART_initStruct.RXThresholdIEn = 1;
    
	UART_initStruct.TXThreshold = 0;
	UART_initStruct.TXThresholdIEn = 0;
    
	UART_initStruct.TimeoutTime = 255;
	UART_initStruct.TimeoutIEn = 1;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}