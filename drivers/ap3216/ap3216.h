#ifndef _AP3216_H_
#define _AP3216_H_

#define AP3216C_I2C_ADDR 0x1E  //

#define AP3216C_REG_SYSCONFIG    0x00
#define AP3216C_REG_IRDATA_LO    0x0A
#define AP3216C_REG_IRDATA_HI    0x0B
#define AP3216C_REG_ALSDATA_LO   0x0C
#define AP3216C_REG_ALSDATA_HI   0x0D
#define AP3216C_REG_PSDATA_LO    0x0E
#define AP3216C_REG_PSDATA_HI    0x0F

extern int ap3216c_init(void);
extern int ap3216c_read_als(void);



#endif