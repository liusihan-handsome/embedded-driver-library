#include "ap3216.h"
#include "iic.h"

#define LOG_TAG "AP3216C"

#include "elog.h"


static int i2c_read_reg(unsigned char reg_addr, unsigned char *data)
{
    IIC_Start();
	  IIC_Write_Byte((AP3216C_I2C_ADDR<<1)&0XFE);
	  //IIC_Write_Byte(reg_addr>>16);
	  IIC_Write_Byte(reg_addr);
	
	  IIC_Start();
	  IIC_Write_Byte((AP3216C_I2C_ADDR<<1)|0X01);
	  *data = IIC_Read_Byte();
	  
	  IIC_NAck();
	  IIC_Stop();
	  
    return 0;
}

static int i2c_write_reg(unsigned char reg_addr, unsigned char value)
{
    
	  IIC_Start();
	  IIC_Write_Byte((AP3216C_I2C_ADDR<<1)&0XFE);
	  IIC_Write_Byte(reg_addr);
	  IIC_Write_Byte(value);
	  IIC_Stop();
	
    return 0;
}

int ap3216c_init(void)
{
	/*
		000: Power down (Default)
		001: ALS function active
		010: PS+IR function active
		011: ALS and PS+IR functions active
		100: SW reset
		101: ALS function once
		110: PS+IR function once
		111: ALS and PS+IR functions once
	*/
	
  i2c_write_reg(AP3216C_REG_SYSCONFIG,0X04);
	HAL_Delay(200);
	i2c_write_reg(AP3216C_REG_SYSCONFIG,0X03);
  HAL_Delay(100);
  return 0;
}	

int ap3216c_read_als(void)
{
 uint16_t als_value = 0;
	
 i2c_read_reg(AP3216C_REG_ALSDATA_LO,((uint8_t *)(&als_value))+1);
 i2c_read_reg(AP3216C_REG_ALSDATA_HI,((uint8_t *)(&als_value))+0);
	
 log_i("als_value is %d",als_value);
 //printf("als_value is %d",als_value);
	
 return als_value;
}
