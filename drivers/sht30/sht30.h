#ifndef __SHT20_H__
#define __SHT20_H__

#include "iic.h"



typedef signed char				s8_t;
typedef unsigned char			u8_t;
typedef signed short			s16_t;
typedef unsigned short			u16_t;
typedef signed int				s32_t;
typedef unsigned int			u32_t;
typedef signed long long		s64_t;
typedef unsigned long long		u64_t;


#define AHT21_SDA_PORT          SDA_PORT
#define AHT21_SDA_PIN           SDA_PIN


#define AHT21_SCL_PORT          SCL_PORT
#define AHT21_SCL_PIN           SCL_PIN


void Delay_N10us(uint32_t t);//延时函数
void SensorDelay_us(uint32_t t);//延时函数
void Delay_4us(void);		//延时函数
void Delay_5us(void);		//延时函数
void Delay_1ms(uint32_t t);	
void AHT20_Clock_Init(void);		//延时函数
void SDA_Pin_Output_High(void)  ; //将PB15配置为输出 ， 并设置为高电平， PB15作为I2C的SDA
void SDA_Pin_Output_Low(void);  //将P15配置为输出  并设置为低电平
void SDA_Pin_IN_FLOATING(void);  //SDA配置为浮空输入
void SCL_Pin_Output_High(void); //SCL输出高电平，P14作为I2C的SCL
void SCL_Pin_Output_Low(void); //SCL输出低电平
void Init_I2C_SHT30_Sensor(void); //初始化I2C接口,输出为高电平
void I2C_Start(void);		 //I2C主机发送START信号
void AHT20_WR_Byte(uint8_t Byte); //往AHT20写一个字节
uint8_t AHT20_RD_Byte(void);//从AHT20读取一个字节
uint8_t Receive_ACK(void);   //看AHT20是否有回复ACK
void Send_ACK(void)	;	  //主机回复ACK信号
void Send_NOT_ACK(void);	//主机不回复ACK
void Stop_I2C(void);	  //一条协议结束
uint8_t AHT20_Read_Status(void);//读取AHT20的状态寄存器
void AHT20_SendAC(void); //向AHT20发送AC命令
uint8_t Calc_CRC8(uint8_t *message,uint8_t Num);
void AHT20_Read_CTdata(uint32_t *ct); //没有CRC校验，直接读取AHT20的温度和湿度数据
void AHT20_Read_CTdata_crc(uint32_t *ct); //CRC校验后，读取AHT20的温度和湿度数据
void JH_Reset_REG(uint8_t addr);///重置寄存器
void AHT20_Start_Init(void);///上电初始化进入正常测量状态

int SHT30_Get_Data(void);

#endif
