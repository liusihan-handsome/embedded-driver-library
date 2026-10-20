#include "project.h"


/*
(1)串口按照协议要求初始化
(2)接收中断把数据存起来
	一般使用环形缓冲区，收到数据，往环形缓冲区里面写
	
	①串口初始化之前，构建环形缓冲区
	②在串口的接收中断，往环形缓冲区里面写数据
		超时接收中断
		FIFO溢出/达标中断
		
		
	
	
(3)main()函数的while(1)循环种，把收到的数据，解析出来
	使用状态机，从环形缓冲区里面把数据读出来，并解析。


*/
void work(void);
int main(void)
{
	SystemInit();			//初始化系统时钟
	
	SerialInit( 115200 );			//初始化串口
   
	printf("system run...\r\n");
	

	while(1==1)
	{
		//work();
        cmd_line_work();
	}
}
uint8_t sta=0;
#define FILE_HEAD 0xA5
#define FILE_Tail 0x5A
#define FAIL_Length 12
int rx_len1 = 0;
uint8_t recv_buf[FAIL_Length];
uint32_t ret;
void work(void)
{
    uint8_t sum=0;
    uint8_t i;
    switch(sta)
    {
        case 0:
       ret=loopbuf_read(buf,recv_buf,1);
        if(ret==0) break;
        if(recv_buf[0]!=FILE_HEAD) break;
        sta++;
        rx_len1++;
        break;
        case 1:
            ret=loopbuf_read(buf,&recv_buf[rx_len1],FAIL_Length-rx_len1);
            rx_len1+=ret;
        if(rx_len1<FAIL_Length) break;
        if(recv_buf[FAIL_Length-1]!=FILE_Tail)
        {
            sta=0;
            rx_len1=0;
        }
        for(i=0;i<9;i++)
        {
            sum+=recv_buf[1+i];
        }
        if(sum!=recv_buf[FAIL_Length-2])
        {
            sta=0;
            rx_len1=0;
        }
        
        cmd_analysis(recv_buf);
            sta=0;
            rx_len1=0;
        break;
        default:
            break;
    }
}
