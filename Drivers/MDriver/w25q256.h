#ifndef __W25QXX_H__
#define __W25QXX_H__
 
#include <stdbool.h>
#include "spi.h"
 
#define SPI_FLASH_PageSize                  	256
#define SPI_FLASH_PerWritePageSize          	256
 
#define W25X_WriteEnable		              	0x06 
#define W25X_WriteDisable		              	0x04 
#define W25X_ReadStatusReg1		              	0x05 
#define W25X_WriteStatusReg1	              	0x01 
#define W25X_ReadData			              	0x03 
#define W25X_FastReadData		              	0x0B 
#define W25X_FastReadDual		              	0x3B 
#define W25X_PageProgram		              	0x02 
#define W25X_BlockErase			              	0xD8 
#define W25X_SectorErase		              	0x20 
#define W25X_ChipErase			              	0xC7 
#define W25X_PowerDown			              	0xB9 
#define W25X_ReleasePowerDown	              	0xAB 
#define W25X_DeviceID			              	0xAB 
#define W25X_ManufactDeviceID   	          	0x90 
#define W25X_JedecDeviceID		              	0x9F 
	
#define WIP_Flag                              	0x01  /* Write In Progress (WIP) flag */
	
#define W25QXX_DUMMY_BYTE					  	0xFF
 
#pragma pack(1)
 
typedef enum
{
	W25Q10=1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,
	
}W25QXX_ID_t;
 
typedef struct
{
	W25QXX_ID_t	ID;
	uint8_t		UniqID[8];
	uint16_t	PageSize;
	uint32_t	PageCount;
	uint32_t	SectorSize;
	uint32_t	SectorCount;
	uint32_t	BlockSize;
	uint32_t	BlockCount;
	uint32_t	CapacityInKiloByte;
	uint8_t		StatusRegister1;
	uint8_t		StatusRegister2;
	uint8_t		StatusRegister3;	
	uint8_t		Lock;
	
}w25qxx_t;
 
#pragma pack()
 
extern w25qxx_t	w25qxx;
 
/************************************************ÓÃ»§API*******************************************/
bool		W25qxx_Init(void);
 
uint32_t W25qxx_ReadID(void);

void		W25qxx_EraseChip(void);
void 		W25qxx_EraseSector(uint32_t SectorAddr);
void 		W25qxx_EraseBlock(uint32_t BlockAddr);
 
uint32_t	W25qxx_PageToSector(uint32_t PageAddress);
uint32_t	W25qxx_PageToBlock(uint32_t PageAddress);
uint32_t	W25qxx_SectorToBlock(uint32_t SectorAddress);
uint32_t	W25qxx_SectorToPage(uint32_t SectorAddress);
uint32_t	W25qxx_BlockToPage(uint32_t BlockAddress);
 
void 		W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr);
void 		W25qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void 		W25qxx_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
 
void 		W25qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
 
#endif