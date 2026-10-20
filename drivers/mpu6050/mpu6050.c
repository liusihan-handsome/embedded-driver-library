#include "mpu6050.h"

#define MPU6050_ADDRESS 0x68

void i2c_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;

    I2C_SCL_CLK();
    I2C_SDA_CLK();
    
    gpio_initstruct.Pin = I2C_SCL_PIN;          
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_OD;             
    gpio_initstruct.Pull = GPIO_PULLUP;                     
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;           
    HAL_GPIO_Init(I2C_SCL_PORT, &gpio_initstruct);
    
    gpio_initstruct.Pin = I2C_SDA_PIN;          
    HAL_GPIO_Init(I2C_SDA_PORT, &gpio_initstruct);
}

void i2c_start(void)
{
    I2C_SDA_SET();
    I2C_SCL_SET();
    I2C_SDA_RESET();
    I2C_SCL_RESET();
}

void i2c_stop(void)
{
    I2C_SDA_RESET();
    I2C_SCL_SET();
    I2C_SDA_SET();
}

void i2c_send_ack(uint8_t ack)
{
    if(ack == 1)
        I2C_SDA_SET();
    else
        I2C_SDA_RESET();
    
    I2C_SCL_SET();
    I2C_SCL_RESET();
}

uint8_t i2c_receive_ack(void)
{
    uint8_t ack;
    I2C_SDA_SET();
    I2C_SCL_SET();
    ack = I2C_SDA_VALUE();
    I2C_SCL_RESET();
    return ack;
}

void i2c_send_byte(uint8_t data)
{
    uint8_t i, tmp;
    tmp = data;
    
    for(i = 0; i < 8; i++)
    {
        if((tmp & 0x80) == 0x80)
            I2C_SDA_SET();
        else
            I2C_SDA_RESET();
        tmp = tmp << 1;
        I2C_SCL_SET();
        I2C_SCL_RESET();
    }
}

uint8_t i2c_receive_byte(void)
{
    uint8_t i = 0, data = 0;
    I2C_SDA_SET();
    for(i = 0; i < 8; i++)
    {
        I2C_SCL_SET();
        if(I2C_SDA_VALUE() == GPIO_PIN_SET)
            data |= (0x80 >> i);
        I2C_SCL_RESET();
    }
    
    return data;
}

void mpu6050_write_register(uint8_t address, uint8_t data)
{
    //I2C起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x00);
    //等待从机响应
    i2c_receive_ack();
    //发送寄存器地址
    i2c_send_byte(address);
    //等待从机响应
    i2c_receive_ack();
    //发送写入的数据
    i2c_send_byte(data);
    //等待从机响应
    i2c_receive_ack();
    //I2C终止信号
    i2c_stop();
}

uint8_t mpu6050_read_register(uint8_t address)
{
    uint8_t data = 0;
    
    //I2C起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x00);
    //等待从机响应
    i2c_receive_ack();
    //发送寄存器地址
    i2c_send_byte(address);
    //等待从机响应
    i2c_receive_ack();
    
    //I2C再次起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x01);
    //等待从机响应
    i2c_receive_ack();
    //接收从机数据
    data = i2c_receive_byte();
    //发送ack 1，停止数据传输
    i2c_send_ack(1);
    //I2C终止信号
    i2c_stop();
    
    return data;
}

void mpu6050_write_len(uint8_t reg_addr, uint8_t len, uint8_t *data)
{
    uint8_t i = 0;
    
    //I2C起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x00);
    //等待从机响应
    i2c_receive_ack();
    //发送寄存器地址
    i2c_send_byte(reg_addr);
    //等待从机响应
    i2c_receive_ack();
    
    for(i = 0; i < len; i++)
    {
        //发送写入的数据
        i2c_send_byte(data[i]);
        //等待从机响应
        i2c_receive_ack();
    }
    
    //I2C终止信号
    i2c_stop();
}

void mpu6050_read_len(uint8_t reg_addr, uint8_t len, uint8_t *data)
{
    //I2C起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x00);
    //等待从机响应
    i2c_receive_ack();
    //发送寄存器地址
    i2c_send_byte(reg_addr);
    //等待从机响应
    i2c_receive_ack();
    
    //I2C再次起始信号
    i2c_start();
    //发送从机地址
    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x01);
    //等待从机响应
    i2c_receive_ack();
    
    while(len--)
    {
        //接收从机数据
        *(data++) = i2c_receive_byte();
        if(len == 0)
            i2c_send_ack(1);
        else
            i2c_send_ack(0);
    }
    
    //I2C终止信号
    i2c_stop();
}

void mpu6050_get_data(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                      int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t DataH, DataL;
    
    DataH = mpu6050_read_register(MPU6050_ACCEL_XOUT_H);    //读取加速度计X轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_ACCEL_XOUT_L);    //读取加速度计X轴的低8位数据
    *AccX = (DataH << 8) | DataL;                           //数据拼接，通过输出参数返回
    
    DataH = mpu6050_read_register(MPU6050_ACCEL_YOUT_H);    //读取加速度计Y轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_ACCEL_YOUT_L);    //读取加速度计Y轴的低8位数据
    *AccY = (DataH << 8) | DataL;                           //数据拼接，通过输出参数返回
    
    DataH = mpu6050_read_register(MPU6050_ACCEL_ZOUT_H);    //读取加速度计Z轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_ACCEL_ZOUT_L);    //读取加速度计Z轴的低8位数据
    *AccZ = (DataH << 8) | DataL;                           //数据拼接，通过输出参数返回
    
    DataH = mpu6050_read_register(MPU6050_GYRO_XOUT_H);     //读取陀螺仪X轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_GYRO_XOUT_L);     //读取陀螺仪X轴的低8位数据
    *GyroX = (DataH << 8) | DataL;                          //数据拼接，通过输出参数返回
    
    DataH = mpu6050_read_register(MPU6050_GYRO_YOUT_H);     //读取陀螺仪Y轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_GYRO_YOUT_L);     //读取陀螺仪Y轴的低8位数据
    *GyroY = (DataH << 8) | DataL;                          //数据拼接，通过输出参数返回
    
    DataH = mpu6050_read_register(MPU6050_GYRO_ZOUT_H);     //读取陀螺仪Z轴的高8位数据
    DataL = mpu6050_read_register(MPU6050_GYRO_ZOUT_L);     //读取陀螺仪Z轴的低8位数据
    *GyroZ = (DataH << 8) | DataL;                          //数据拼接，通过输出参数返回
}

void mpu6050_init(void)
{
    i2c_init();
    
    mpu6050_write_register(MPU6050_PWR_MGMT_1, 0x01);           //电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
    mpu6050_write_register(MPU6050_PWR_MGMT_2, 0x00);           //电源管理寄存器2，保持默认值0，所有轴均不待机
    mpu6050_write_register(MPU6050_SMPLRT_DIV, 0x09);           //采样率分频寄存器，配置采样率
    mpu6050_write_register(MPU6050_CONFIG, 0x06);               //配置寄存器，配置DLPF
    mpu6050_write_register(MPU6050_GYRO_CONFIG, 0x18);          //陀螺仪配置寄存器，选择满量程为±2000°/s
    mpu6050_write_register(MPU6050_ACCEL_CONFIG, 0x18);         //加速度计配置寄存器，选择满量程为±16g
}

void mpu6050_test(void)
{
////////////////////////1/////////////////////////////
//    i2c_start();
//    i2c_send_byte((MPU6050_ADDRESS << 1) | 0x00);
//    uint8_t ack = i2c_receive_ack();
//    i2c_stop();
//    
//    printf("ack: %d\r\n", ack);
    
////////////////////////2/////////////////////////////
//    uint8_t id = mpu6050_read_register(0x75);
//    printf("id: %x\r\n", id);
    
////////////////////////3/////////////////////////////
//    mpu6050_write_register(0x6B, 0x00);
//    mpu6050_write_register(0x19, 0xAA);
//    uint8_t data = mpu6050_read_register(0x19);
//    printf("data: %x\r\n", data);
    
////////////////////////4/////////////////////////////
    uint8_t data_read[4] = {0}, data_write[4] = {0x11, 0x22, 0x33, 0x44};
    mpu6050_read_len(MPU6050_SMPLRT_DIV, 4, data_read);
    printf("data_read: %X, %X, %X, %X\r\n", data_read[0], data_read[1], data_read[2], data_read[3]);
    
    mpu6050_write_len(MPU6050_SMPLRT_DIV, 4, data_write);
    mpu6050_read_len(MPU6050_SMPLRT_DIV, 4, data_read);
    printf("data_read: %X, %X, %X, %X\r\n", data_read[0], data_read[1], data_read[2], data_read[3]);
}
