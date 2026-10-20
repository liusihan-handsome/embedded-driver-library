#ifndef __w25q128
#define __w25q128
#include "stdint.h"
#define ManufactDeviceID_CMD    0x90
#define READ_STATU_REGISTER_1   0x05
#define READ_STATU_REGISTER_2   0x35
#define READ_DATA_CMD           0x03
#define WRITE_ENABLE_CMD        0x06
#define WRITE_DISABLE_CMD       0x04
#define SECTOR_ERASE_CMD        0x20
#define CHIP_ERASE_CMD          0xc7
#define PAGE_PROGRAM_CMD        0x02


#define SPI_FLASH_CS_LOW()      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
#define SPI_FLASH_CS_HIGH()     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
extern void W25QXX_Write_Sector(uint8_t* pBuffer, uint32_t nSector);
extern void W25QXX_Read_Sector(uint8_t* pBuffer, uint32_t nSector);
#endif
