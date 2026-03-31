#include "w25q256.h"
#include "w25qxx_config.h"
#include "stm32f1xx_hal.h"
#include "main.h"
 
#if (_W25QXX_DEBUG == 1)
#include <stdio.h>
#endif
 
#define W25QXX_CS_H 			HAL_GPIO_WritePin(W25Q256_CS_GPIO_Port, W25Q256_CS_Pin, GPIO_PIN_SET);
#define W25QXX_CS_L 			HAL_GPIO_WritePin(W25Q256_CS_GPIO_Port, W25Q256_CS_Pin, GPIO_PIN_RESET);
 
w25qxx_t	w25qxx;
 
#if (_W25QXX_USE_FREERTOS == 1)
#define	W25qxx_Delay(delay)		osDelay(delay)
#include "cmsis_os.h"
#else
#define	W25qxx_Delay(delay) for(uint8_t i=0;i<255;i++);;
#endif
 
//###################################################################################################################
uint8_t	W25qxx_Spi(uint8_t	Data)
{
	uint8_t	ret;
	HAL_SPI_TransmitReceive(&_W25QXX_SPI, &Data, &ret, 1, 100);
	return ret;	
}
//###################################################################################################################
uint32_t W25qxx_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
	
	W25QXX_CS_L;
	W25qxx_Spi(W25X_JedecDeviceID);
	Temp0 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	Temp1 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	Temp2 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	W25QXX_CS_H;
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	
	return Temp;
}
//###################################################################################################################
void W25qxx_ReadUniqID(void)
{
	W25QXX_CS_L;
	W25qxx_Spi(0x4B);
	for(uint8_t	i = 0; i < 4; i++)
	{
		W25qxx_Spi(W25QXX_DUMMY_BYTE);
	}
	for(uint8_t	i = 0; i < 8; i++)
	{
		w25qxx.UniqID[i] = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	}
	W25QXX_CS_H;
}
//###################################################################################################################
void W25qxx_WriteEnable(void)
{
	W25QXX_CS_L;
	W25qxx_Spi(W25X_WriteEnable);
	W25QXX_CS_H;
	W25qxx_Delay(1);
}
//###################################################################################################################
void W25qxx_WriteDisable(void)
{
	W25QXX_CS_L;
	W25qxx_Spi(W25X_WriteDisable);
	W25QXX_CS_H;
	W25qxx_Delay(1);
}
//###################################################################################################################
uint8_t W25qxx_ReadStatusRegister(uint8_t SelectStatusRegister_1_2_3)
{
	uint8_t	status=0;
	W25QXX_CS_L;
	if(SelectStatusRegister_1_2_3 == 1)
	{
		W25qxx_Spi(W25X_ReadStatusReg1);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister1 = status;
	}
	else if(SelectStatusRegister_1_2_3 == 2)
	{
		W25qxx_Spi(0x35);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister2 = status;
	}
	else
	{
		W25qxx_Spi(0x15);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister3 = status;
	}	
	W25QXX_CS_H;
	
	return status;
}
//###################################################################################################################
void W25qxx_WriteStatusRegister(uint8_t	SelectStatusRegister_1_2_3, uint8_t Data)
{
	W25QXX_CS_L;
	if(SelectStatusRegister_1_2_3 == 1)
	{
		W25qxx_Spi(W25X_WriteStatusReg1);
		w25qxx.StatusRegister1 = Data;
	}
	else if(SelectStatusRegister_1_2_3 == 2)
	{
		W25qxx_Spi(0x31);
		w25qxx.StatusRegister2 = Data;
	}
	else
	{
		W25qxx_Spi(0x11);
		w25qxx.StatusRegister3 = Data;
	}
	W25qxx_Spi(Data);
	W25QXX_CS_H;
}
//###################################################################################################################
void W25qxx_WaitForWriteEnd(void)
{
	W25QXX_CS_L;
	W25qxx_Spi(W25X_ReadStatusReg1);
	do
	{
		w25qxx.StatusRegister1 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
		W25qxx_Delay(1);
	}
	while ((w25qxx.StatusRegister1 & WIP_Flag) == 0x01);
	W25QXX_CS_H;
 
}
//###################################################################################################################
bool W25qxx_Init(void)
{
	w25qxx.Lock=1;	
//	while(HAL_GetTick()<100)
//		W25qxx_Delay(1);
	W25QXX_CS_H;
//  W25qxx_Delay(100);
	uint32_t	id;
#if (_W25QXX_DEBUG==1)
	printf("w25qxx Init Begin...\r\n");
#endif
	id = W25qxx_ReadID();
	
#if (_W25QXX_DEBUG==1)
	printf("w25qxx ID:0x%X\r\n",id);
#endif
	switch(id&0x0000FFFF)
	{
		case 0x401A:	// 	w25q512
			w25qxx.ID=W25Q512;
			w25qxx.BlockCount=1024;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q512\r\n");
#endif
		break;
		case 0x4019:	// 	w25q256
			w25qxx.ID=W25Q256;
			w25qxx.BlockCount=512;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q256\r\n");
#endif
		break;
		case 0x4018:	// 	w25q128
			w25qxx.ID=W25Q128;
			w25qxx.BlockCount=256;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q128\r\n");
#endif
		break;
		case 0x4017:	//	w25q64
			w25qxx.ID=W25Q64;
			w25qxx.BlockCount=128;
#if (_W25QXX_DEBUG == 1)
			printf("w25qxx Chip: w25q64\r\n");
#endif
		break;
		case 0x4016:	//	w25q32
			w25qxx.ID=W25Q32;
			w25qxx.BlockCount=64;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q32\r\n");
#endif
		break;
		case 0x4015:	//	w25q16
			w25qxx.ID=W25Q16;
			w25qxx.BlockCount=32;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q16\r\n");
#endif
		break;
		case 0x4014:	//	w25q80
			w25qxx.ID=W25Q80;
			w25qxx.BlockCount=16;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q80\r\n");
#endif
		break;
		case 0x4013:	//	w25q40
			w25qxx.ID=W25Q40;
			w25qxx.BlockCount=8;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q40\r\n");
#endif
		break;
		case 0x4012:	//	w25q20
			w25qxx.ID=W25Q20;
			w25qxx.BlockCount=4;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q20\r\n");
#endif
		break;
		case 0x4011:	//	w25q10
			w25qxx.ID=W25Q10;
			w25qxx.BlockCount=2;
#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q10\r\n");
#endif
		break;
		default:
#if (_W25QXX_DEBUG==1)
				printf("w25qxx Unknown ID\r\n");
#endif
			w25qxx.Lock=0;	
			return false;		
	}		
	w25qxx.PageSize = 256;
	w25qxx.SectorSize = 0x1000;
	w25qxx.SectorCount = w25qxx.BlockCount * 16;
	w25qxx.PageCount = (w25qxx.SectorCount * w25qxx.SectorSize) / w25qxx.PageSize;
	w25qxx.BlockSize = w25qxx.SectorSize*16;
	w25qxx.CapacityInKiloByte = (w25qxx.SectorCount * w25qxx.SectorSize) / 1024;
	W25qxx_ReadUniqID();
	W25qxx_ReadStatusRegister(1);
	W25qxx_ReadStatusRegister(2);
	W25qxx_ReadStatusRegister(3);
#if (_W25QXX_DEBUG == 1)
	printf("w25qxx Page Size: %d Bytes\r\n",w25qxx.PageSize);
	printf("w25qxx Page Count: %d\r\n",w25qxx.PageCount);
	printf("w25qxx Sector Size: %d Bytes\r\n",w25qxx.SectorSize);
	printf("w25qxx Sector Count: %d\r\n",w25qxx.SectorCount);
	printf("w25qxx Block Size: %d Bytes\r\n",w25qxx.BlockSize);
	printf("w25qxx Block Count: %d\r\n",w25qxx.BlockCount);
	printf("w25qxx Capacity: %d KiloBytes\r\n",w25qxx.CapacityInKiloByte);
	printf("w25qxx Init Done\r\n");
#endif
	w25qxx.Lock = 0;	
	return true;
}	
//###################################################################################################################
void W25qxx_EraseChip(void)
{
	while(w25qxx.Lock==1)
	{
		W25qxx_Delay(1);
	}
	w25qxx.Lock=1;	
#if (_W25QXX_DEBUG == 1)
	uint32_t	StartTime=HAL_GetTick();	
	printf("w25qxx EraseChip Begin...\r\n");
#endif
	W25qxx_WriteEnable();
	W25QXX_CS_L;
	W25qxx_Spi(W25X_ChipErase);
	W25QXX_CS_H;
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG == 1)
	printf("w25qxx EraseBlock done after %d ms!\r\n",HAL_GetTick()-StartTime);
#endif
	w25qxx.Lock = 0;	
}
//###################################################################################################################
void W25qxx_EraseSector(uint32_t SectorAddr)
{
	while(w25qxx.Lock == 1)
	{
		W25qxx_Delay(1);
	}
	w25qxx.Lock = 1;	
#if (_W25QXX_DEBUG == 1)
	uint32_t	StartTime=HAL_GetTick();	
	printf("w25qxx EraseSector %d Begin...\r\n",SectorAddr);
#endif
	W25qxx_WriteEnable();
	W25qxx_WaitForWriteEnd();
	W25QXX_CS_L;
	W25qxx_Spi(W25X_SectorErase);
	if(w25qxx.ID == W25Q256)
	{
		W25qxx_Spi((SectorAddr & 0xFF000000) >> 24);
	}
	W25qxx_Spi((SectorAddr & 0xFF0000) >> 16);
	W25qxx_Spi((SectorAddr & 0xFF00) >> 8);
	W25qxx_Spi(SectorAddr & 0xFF);
	W25QXX_CS_H;
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseSector done after %d ms\r\n",HAL_GetTick()-StartTime);
#endif
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
	while(w25qxx.Lock==1)
	{
		W25qxx_Delay(1);
	}
	w25qxx.Lock=1;	
#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock %d Begin...\r\n",BlockAddr);
	W25qxx_Delay(100);
	uint32_t StartTime=HAL_GetTick();	
#endif
	W25qxx_WriteEnable();
	W25qxx_WaitForWriteEnd();
	BlockAddr = BlockAddr * w25qxx.SectorSize * 16;
	W25QXX_CS_L;
	W25qxx_Spi(W25X_BlockErase);
	if(w25qxx.ID == W25Q256)
	{
		W25qxx_Spi((BlockAddr & 0xFF000000) >> 24);
	}
	W25qxx_Spi((BlockAddr & 0xFF0000) >> 16);
	W25qxx_Spi((BlockAddr & 0xFF00) >> 8);
	W25qxx_Spi(BlockAddr & 0xFF);
	W25QXX_CS_H;
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock done after %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
#endif
	w25qxx.Lock=0;
}
//###################################################################################################################
uint32_t W25qxx_PageToSector(uint32_t PageAddress)
{
	return ((PageAddress * w25qxx.PageSize) / w25qxx.SectorSize);
}
//###################################################################################################################
uint32_t W25qxx_PageToBlock(uint32_t PageAddress)
{
	return ((PageAddress * w25qxx.PageSize) / w25qxx.BlockSize);
}
//###################################################################################################################
uint32_t W25qxx_SectorToBlock(uint32_t SectorAddress)
{
	return ((SectorAddress * w25qxx.SectorSize) / w25qxx.BlockSize);
}
//###################################################################################################################
uint32_t W25qxx_SectorToPage(uint32_t SectorAddress)
{
	return (SectorAddress * w25qxx.SectorSize) / w25qxx.PageSize;
}
//###################################################################################################################
uint32_t W25qxx_BlockToPage(uint32_t BlockAddress)
{
	return (BlockAddress * w25qxx.BlockSize) / w25qxx.PageSize;
}
//###################################################################################################################
void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr)
{
	while(w25qxx.Lock==1)
	{
		W25qxx_Delay(1);
	}
	w25qxx.Lock = 1;
#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx WriteByte 0x%02X at address %d begin...",pBuffer,WriteAddr_inBytes);
#endif
	W25qxx_WriteEnable();
	W25qxx_WaitForWriteEnd();
	W25QXX_CS_L;
	W25qxx_Spi(W25X_PageProgram);
	if(w25qxx.ID == W25Q256)
	{
		W25qxx_Spi((WriteAddr & 0xFF000000) >> 24);
	}
	W25qxx_Spi((WriteAddr & 0xFF0000) >> 16);
	W25qxx_Spi((WriteAddr & 0xFF00) >> 8);
	W25qxx_Spi(WriteAddr & 0xFF);
	W25qxx_Spi(pBuffer);
	W25QXX_CS_H;
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG==1)
	printf("w25qxx WriteByte done after %d ms\r\n",HAL_GetTick()-StartTime);
#endif
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	while(w25qxx.Lock==1)
	{
		W25qxx_Delay(1);
	}
	w25qxx.Lock = 1;
	W25qxx_WriteEnable();
	W25qxx_WaitForWriteEnd();
	W25QXX_CS_L;
	W25qxx_Spi(W25X_PageProgram);
	if(w25qxx.ID == W25Q256)
	{
		W25qxx_Spi((WriteAddr & 0xFF000000) >> 24);
	}
	W25qxx_Spi((WriteAddr & 0xFF0000) >> 16);
	W25qxx_Spi((WriteAddr & 0xFF00) >> 8);
	W25qxx_Spi(WriteAddr & 0xFF);
	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}
	while (NumByteToWrite--)
	{
		W25qxx_Spi(*pBuffer);
		pBuffer++;
	}
	W25QXX_CS_H;
	W25qxx_WaitForWriteEnd();
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
 
	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
	if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			W25qxx_WritePage(pBuffer, WriteAddr, NumByteToWrite);
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			while (NumOfPage--)
			{
				W25qxx_WritePage(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			W25qxx_WritePage(pBuffer, WriteAddr, NumOfSingle);
		}
	}
	else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
			{
				temp = NumOfSingle - count;
				W25qxx_WritePage(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;
				W25qxx_WritePage(pBuffer, WriteAddr, temp);
			}
			else
			{
				W25qxx_WritePage(pBuffer, WriteAddr, NumByteToWrite);
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
			W25qxx_WritePage(pBuffer, WriteAddr, count);
			WriteAddr +=  count;
			pBuffer += count;
			while (NumOfPage--)
			{
				W25qxx_WritePage(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			if (NumOfSingle != 0)
			{
				W25qxx_WritePage(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}
//###################################################################################################################
void W25qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	W25QXX_CS_L;
	W25qxx_Spi(W25X_ReadData);
	if(w25qxx.ID == W25Q256)
	{
		W25qxx_Spi((ReadAddr & 0xFF000000) >> 24);
	}
	W25qxx_Spi((ReadAddr & 0xFF0000) >> 16);
	W25qxx_Spi((ReadAddr& 0xFF00) >> 8);
	W25qxx_Spi(ReadAddr & 0xFF);
	while (NumByteToRead--)
	{
		*pBuffer = W25qxx_Spi(W25QXX_DUMMY_BYTE);
		pBuffer++;
	}
	W25QXX_CS_H;
}
//###################################################################################################################