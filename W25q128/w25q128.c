#include "w25q128.h"

#include "stdint.h"
#include "stm32f1xx_hal.h"

#include "shell.h"

#include "string.h"
#include "stdlib.h"
#include "elog.h"

extern SPI_HandleTypeDef hspi2; 

uint8_t sector_buf[4096] = {0};

/**
 * @brief    SPI发送指定长度的数据
 * @param    buf  —— 发送数据缓冲区首地址
 * @param    size —— 要发送数据的字节数
 * @retval   成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Transmit(uint8_t* send_buf, uint16_t size)
{
    return HAL_SPI_Transmit(&hspi2, send_buf, size, 100);
}
/**
 * @brief   SPI接收指定长度的数据
 * @param   buf  —— 接收数据缓冲区首地址
 * @param   size —— 要接收数据的字节数
 * @retval  成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Receive(uint8_t* recv_buf, uint16_t size)
{
   return HAL_SPI_Receive(&hspi2, recv_buf, size, 100);
}
/**
 * @brief   SPI在发送数据的同时接收指定长度的数据
 * @param   send_buf  —— 接收数据缓冲区首地址
 * @param   recv_buf  —— 接收数据缓冲区首地址
 * @param   size —— 要发送/接收数据的字节数
 * @retval  成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_TransmitReceive(uint8_t* send_buf, uint8_t* recv_buf, uint16_t size)
{
   return HAL_SPI_TransmitReceive(&hspi2, send_buf, recv_buf, size, 100);
}

/*等待超时时间*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;   
/**
  * @brief  等待超时回调函数
  * @param  None.
  * @retval None.
  */
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
  /* 等待超时后的处理,输出错误信息 */
  // printf("SPI 等待超时!errorCode = %d",errorCode);
  return 0;
}
 /**
  * @brief  使用SPI发送一个字节的数据
  * @param  byte：要发送的数据
  * @retval 返回接收到的数据
  */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
  SPITimeout = SPIT_FLAG_TIMEOUT;

  /* 等待发送缓冲区为空，TXE事件 */
  while (__HAL_SPI_GET_FLAG( &hspi2, SPI_FLAG_TXE ) == RESET)
   {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
   }

  /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
  WRITE_REG(hspi2.Instance->DR, byte);

  SPITimeout = SPIT_FLAG_TIMEOUT;

  /* 等待接收缓冲区非空，RXNE事件 */
  while (__HAL_SPI_GET_FLAG( &hspi2, SPI_FLAG_RXNE ) == RESET)
   {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
   }

  /* 读取数据寄存器，获取接收缓冲区数据 */
  return READ_REG(hspi2.Instance->DR);
}

//=======================================================================

uint16_t W25Q128_ReadID(void)
{
 uint8_t send_cmd[4] = {ManufactDeviceID_CMD,0X00,0X00,0X00};
 uint8_t recv_data[2];
 
 SPI_FLASH_CS_LOW();
 
 SPI_Transmit(send_cmd,4);
 
 SPI_Receive(recv_data,2);
return ((uint16_t*)recv_data)[0];
 
}

static uint8_t W25QXX_ReadSR()
{
 uint8_t result = 0; 
 uint8_t send_buf[4] = {READ_STATU_REGISTER_1,0x00,0x00,0x00};

 SPI_FLASH_CS_LOW();
    
 if(HAL_OK == SPI_Transmit(send_buf, 4)) 
 {
  if(HAL_OK == SPI_Receive(&result, 1)) 
  {
   SPI_FLASH_CS_HIGH();
		
   return result;
  }
 }
 
 SPI_FLASH_CS_HIGH();

 return 0;
}

static void W25QXX_Wait_Busy(void)
{
 while((W25QXX_ReadSR() & 0x01) == 0x01);
}

void W25QXX_Write_Enable(void)
{
 uint8_t cmd = WRITE_ENABLE_CMD;
    
 SPI_FLASH_CS_LOW();
    
 SPI_Transmit(&cmd, 1);
    
 SPI_FLASH_CS_HIGH();
    
 W25QXX_Wait_Busy();
}

void W25QXX_Erase_Sector(uint32_t sector_addr)
{   
 W25QXX_Write_Enable();  //擦除操作即写入0xFF，需要开启写使能
 W25QXX_Wait_Busy();     //等待写使能完成
 
 /* 使能片选 */
 SPI_FLASH_CS_LOW();
 
 /* 发送扇区擦除指令*/
 SPI_FLASH_SendByte(SECTOR_ERASE_CMD);
	
 /*发送擦除扇区地址的高位*/
 SPI_FLASH_SendByte((sector_addr & 0xFF0000) >> 16);
	
 /* 发送擦除扇区地址的中位 */
 SPI_FLASH_SendByte((sector_addr & 0xFF00) >> 8);
	
 /* 发送擦除扇区地址的低位 */
 SPI_FLASH_SendByte(sector_addr & 0xFF);
 
 /* 取消片选 */
 SPI_FLASH_CS_HIGH();
 
 W25QXX_Wait_Busy();       //等待扇区擦除完成
}

void W25QXX_Read_Sector(uint8_t* pBuffer, uint32_t nSector)
{  
 uint32_t ReadAddr = nSector << 12;	 //`nSector << 12` 等价于 `nSector * 4096`
	
 W25QXX_Wait_Busy();
    
 /* 选择FLASH: CS低电平 */
 SPI_FLASH_CS_LOW();

 /* 发送 读 指令 */
 uint8_t cmd = READ_DATA_CMD;
 SPI_Transmit(&cmd, 1);
    
 uint8_t addr;
 HAL_StatusTypeDef status;
 /* 发送 读 地址高位 */
 addr = (ReadAddr & 0xFF0000) >> 16;
 status = SPI_Transmit(&addr, 1);
 /* 发送 读 地址中位 */
 addr = (ReadAddr& 0xFF00) >> 8;
 status = SPI_Transmit(&addr, 1);
 /* 发送 读 地址低位 */
 addr = ReadAddr & 0xFF;
 status = SPI_Transmit(&addr, 1);
    
 if(HAL_OK == status)
 {
  SPI_Receive(pBuffer, 4096);
 }

 /* 停止信号 FLASH: CS 高电平 */
 SPI_FLASH_CS_HIGH();
}


void W25QXX_PageProgram(uint8_t* dat, uint32_t WriteAddr)
{
 uint8_t cmd = PAGE_PROGRAM_CMD;

 W25QXX_Write_Enable();
 
 /* 使能片选 */
 SPI_FLASH_CS_LOW();
 
 SPI_Transmit(&cmd, 1);

 uint8_t addr;
 HAL_StatusTypeDef status;
	
 /* 发送 地址高位 */
 addr = (WriteAddr & 0xFF0000) >> 16;
 status = SPI_Transmit(&addr, 1);
	
 /* 发送 地址中位 */
 addr = (WriteAddr & 0xFF00) >> 8;
 status = SPI_Transmit(&addr, 1);
	
 /* 发送 地址低位 */
 addr = WriteAddr & 0xFF;
 status = SPI_Transmit(&addr, 1);
 
 SPI_Transmit(dat, 256);
    
 /* 取消片选 */
 SPI_FLASH_CS_HIGH();
    
 W25QXX_Wait_Busy();
}


void W25QXX_Write_Sector(uint8_t* pBuffer, uint32_t nSector)
{
 uint32_t WriteAddr = nSector << 12;
 uint32_t i = 0;	
 
 for(i=0;i<16;i++)
  W25QXX_PageProgram(pBuffer, WriteAddr+i*256);

}

void sfrd(void)
{
 uint16_t id = 0;	
 id = W25Q128_ReadID();
	
 printf("chipid = %04X\r\n",id);
}

ZNS_CMD_EXPORT(sfrd,read spiflash chipid)

void sfes(int argc,char **argv)
{
	uint32_t addr = atoi(argv[1]);
	
	printf("To erase sector No.(%d)\r\n",addr);
	
	printf("erase sector addr : %d\r\n",addr<<12);
	
	W25QXX_Erase_Sector(addr<<12);
}

ZNS_CMD_EXPORT(sfes,erase sector with sector_no 0~4095)

void sfrs(int argc,char **argv)
{
	uint32_t addr = atoi(argv[1]);
	
	printf("To read sector No.(%d)\r\n",addr);
	
	printf("read sector addr : %d\r\n",addr<<12);
	
	W25QXX_Read_Sector(sector_buf,addr<<12);
	
	elog_hexdump("",16,sector_buf,4096);
}

ZNS_CMD_EXPORT(sfrs,read sector with sector_no 0~4095)

void sfws(int argc,char **argv)
{
	uint32_t addr = atoi(argv[1]);
	uint32_t i = 0;
	
	printf("To write sector No.(%d)\r\n",addr);
	
	printf("write sector addr : %d\r\n",addr<<12);
	
	for(i=0;i<4096;i++) sector_buf[i] = i;
	
	W25QXX_Write_Sector(sector_buf,addr<<12);

}

ZNS_CMD_EXPORT(sfws,write sector with sector_no 0~4095)

