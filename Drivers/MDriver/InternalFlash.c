/*
 * InternalFlash.c - STM32F103C8T6 Internal Flash API Implementation
 */

#include "InternalFlash.h"

/* 超时时间定义 */
#define INTFLASH_TIMEOUT_VALUE     10000

/* ============================================================================
 * 基础Flash操作函数
 * ============================================================================ */

HAL_StatusTypeDef INTFLASH_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* 解锁Flash */
    HAL_FLASH_Unlock();
    
    /* 清除所有标志位 */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    
    return status;
}

void INTFLASH_DeInit(void)
{
    /* 锁定Flash */
    HAL_FLASH_Lock();
}

uint8_t INTFLASH_ErasePage(uint32_t page_num)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    HAL_StatusTypeDef status;
    
    /* 检查页号有效性 */
    if (page_num >= INTFLASH_PAGE_NUM)
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 配置擦除参数 - STM32F1使用PageAddress而不是Page */
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = INTFLASH_GetPageAddress(page_num);
    erase_init.NbPages = 1;
    
    /* 执行擦除 */
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    
    if (status != HAL_OK)
    {
        return INTFLASH_ERROR_ERASE;
    }
    
    return INTFLASH_OK;
}

uint8_t INTFLASH_ErasePageByAddr(uint32_t address)
{
    uint32_t page_num = INTFLASH_GetPageNumber(address);
    
    if (page_num >= INTFLASH_PAGE_NUM)
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    return INTFLASH_ErasePage(page_num);
}

uint8_t INTFLASH_ErasePages(uint32_t start_page, uint32_t page_count)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    HAL_StatusTypeDef status;
    uint32_t i;
    
    /* 检查参数有效性 */
    if (start_page >= INTFLASH_PAGE_NUM || 
        (start_page + page_count) > INTFLASH_PAGE_NUM)
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 逐页擦除（STM32F1不支持一次性擦除多页） */
    for (i = 0; i < page_count; i++)
    {
        erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_init.PageAddress = INTFLASH_GetPageAddress(start_page + i);
        erase_init.NbPages = 1;
        
        status = HAL_FLASHEx_Erase(&erase_init, &page_error);
        
        if (status != HAL_OK)
        {
            return INTFLASH_ERROR_ERASE;
        }
    }
    
    return INTFLASH_OK;
}

uint8_t INTFLASH_EraseUserArea(void)
{
    uint32_t start_page = INTFLASH_GetPageNumber(INTFLASH_USER_START_ADDR);
    uint32_t end_page = INTFLASH_GetPageNumber(INTFLASH_USER_END_ADDR);
    uint32_t page_count = end_page - start_page + 1;
    
    return INTFLASH_ErasePages(start_page, page_count);
}

uint8_t INTFLASH_WriteWord(uint32_t address, uint32_t data)
{
    HAL_StatusTypeDef status;
    
    /* 检查地址有效性 */
    if (!INTFLASH_IsAddressValid(address))
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 检查地址对齐 */
    if (address & 0x03)
    {
        return INTFLASH_ERROR_UNALIGNED;
    }
    
    /* 写入数据 */
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    
    if (status != HAL_OK)
    {
        return INTFLASH_ERROR_WRITE;
    }
    
    /* 验证写入 */
    if (INTFLASH_ReadWord(address) != data)
    {
        return INTFLASH_ERROR_WRITE;
    }
    
    return INTFLASH_OK;
}

uint8_t INTFLASH_WriteHalfWord(uint32_t address, uint16_t data)
{
    HAL_StatusTypeDef status;
    
    /* 检查地址有效性 */
    if (!INTFLASH_IsAddressValid(address))
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 检查地址对齐 */
    if (address & 0x01)
    {
        return INTFLASH_ERROR_UNALIGNED;
    }
    
    /* 写入数据 */
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
    
    if (status != HAL_OK)
    {
        return INTFLASH_ERROR_WRITE;
    }
    
    /* 验证写入 */
    if (INTFLASH_ReadHalfWord(address) != data)
    {
        return INTFLASH_ERROR_WRITE;
    }
    
    return INTFLASH_OK;
}

uint8_t INTFLASH_WriteByte(uint32_t address, uint8_t data)
{
    uint16_t halfword_data;
    
    /* 检查地址有效性 */
    if (!INTFLASH_IsAddressValid(address))
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* Flash不支持字节写入，使用半字写入 */
    halfword_data = (uint16_t)data;
    
    return INTFLASH_WriteHalfWord(address, halfword_data);
}

uint8_t INTFLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size)
{
    uint32_t current_addr = address;
    const uint8_t *current_data = data;
    uint32_t remaining = size;
    uint8_t result;
    uint32_t word_data;
    
    /* 检查地址有效性 */
    if (!INTFLASH_IsAddressValid(address) || 
        (address + size) > INTFLASH_USER_END_ADDR + 1)
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 检查大小 */
    if (size == 0)
    {
        return INTFLASH_OK;
    }
    
    /* 处理未对齐的起始部分（字节和半字） */
    while ((remaining > 0) && (current_addr & 0x03))
    {
        result = INTFLASH_WriteByte(current_addr, *current_data);
        if (result != INTFLASH_OK)
        {
            return result;
        }
        current_addr++;
        current_data++;
        remaining--;
    }
    
    /* 以字为单位写入 */
    while (remaining >= 4)
    {
        /* 组装32位数据 */
        word_data = (uint32_t)current_data[0] |
                    ((uint32_t)current_data[1] << 8) |
                    ((uint32_t)current_data[2] << 16) |
                    ((uint32_t)current_data[3] << 24);
        
        result = INTFLASH_WriteWord(current_addr, word_data);
        if (result != INTFLASH_OK)
        {
            return result;
        }
        
        current_addr += 4;
        current_data += 4;
        remaining -= 4;
    }
    
    /* 处理剩余的字节 */
    while (remaining > 0)
    {
        result = INTFLASH_WriteByte(current_addr, *current_data);
        if (result != INTFLASH_OK)
        {
            return result;
        }
        current_addr++;
        current_data++;
        remaining--;
    }
    
    return INTFLASH_OK;
}

uint8_t INTFLASH_ReadBuffer(uint32_t address, uint8_t *buffer, uint32_t size)
{
    uint32_t i;
    
    /* 检查地址有效性 */
    if (!INTFLASH_IsAddressValid(address))
    {
        return INTFLASH_ERROR_ADDR;
    }
    
    /* 读取数据 */
    for (i = 0; i < size; i++)
    {
        buffer[i] = INTFLASH_ReadByte(address + i);
    }
    
    return INTFLASH_OK;
}

uint32_t INTFLASH_ReadWord(uint32_t address)
{
    return *(__IO uint32_t *)address;
}

uint16_t INTFLASH_ReadHalfWord(uint32_t address)
{
    return *(__IO uint16_t *)address;
}

uint8_t INTFLASH_ReadByte(uint32_t address)
{
    return *(__IO uint8_t *)address;
}

/* ============================================================================
 * Flash保护和管理函数
 * ============================================================================ */

uint8_t INTFLASH_IsAddressValid(uint32_t address)
{
    return (address >= INTFLASH_BASE_ADDR && 
            address <= INTFLASH_USER_END_ADDR);
}

uint8_t INTFLASH_IsUserAreaAddress(uint32_t address)
{
    return (address >= INTFLASH_USER_START_ADDR && 
            address <= INTFLASH_USER_END_ADDR);
}

uint32_t INTFLASH_GetPageNumber(uint32_t address)
{
    if (!INTFLASH_IsAddressValid(address))
    {
        return 0xFFFFFFFF;
    }
    
    return (address - INTFLASH_BASE_ADDR) / INTFLASH_PAGE_SIZE;
}

uint32_t INTFLASH_GetPageAddress(uint32_t page_num)
{
    if (page_num >= INTFLASH_PAGE_NUM)
    {
        return 0xFFFFFFFF;
    }
    
    return INTFLASH_BASE_ADDR + (page_num * INTFLASH_PAGE_SIZE);
}

uint8_t INTFLASH_VerifyData(uint32_t address, const uint8_t *data, uint32_t size)
{
    uint32_t i;
    uint8_t read_data;
    
    for (i = 0; i < size; i++)
    {
        read_data = INTFLASH_ReadByte(address + i);
        if (read_data != data[i])
        {
            return 0;
        }
    }
    
    return 1;
}

uint32_t INTFLASH_GetFreeSpace(void)
{
    uint32_t used = 0;
    uint32_t address;
    uint8_t found = 0;
    
    /* 简单查找最后一个非0xFF的地址 */
    for (address = INTFLASH_USER_START_ADDR; address <= INTFLASH_USER_END_ADDR; address++)
    {
        if (INTFLASH_ReadByte(address) != 0xFF)
        {
            used = address - INTFLASH_USER_START_ADDR + 1;
            found = 1;
        }
    }
    
    if (!found)
    {
        used = 0;
    }
    
    return (INTFLASH_USER_END_ADDR - INTFLASH_USER_START_ADDR + 1) - used;
}

uint8_t INTFLASH_IsEmpty(uint32_t address, uint32_t size)
{
    uint32_t i;
    
    for (i = 0; i < size; i++)
    {
        if (INTFLASH_ReadByte(address + i) != 0xFF)
        {
            return 0;
        }
    }
    
    return 1;
}