/*
 * InternalFlash.h - STM32F103C8T6 Internal Flash API
 * 适用于STM32F103C8T6 (64KB Flash, 20KB RAM)
 */

#ifndef INTERNALFLASH_H
#define INTERNALFLASH_H

#include "stm32f1xx_hal.h"

/* Flash页大小定义（避免与HAL库冲突） */
#define INTFLASH_PAGE_SIZE         1024    /* STM32F103C8T6每页1KB */
#define INTFLASH_PAGE_NUM          64      /* 总页数64页 (64KB) */

/* Flash起始地址 */
#define INTFLASH_BASE_ADDR         0x08000000

/* 用户数据存储起始地址（避开程序代码） */
#define INTFLASH_USER_START_ADDR   0x0800F000  /* 最后4KB区域 (60KB-64KB) */
#define INTFLASH_USER_END_ADDR     0x0800FFFF  /* 结束地址 */

/* 错误码定义（避免与HAL库冲突） */
#define INTFLASH_OK                0x00
#define INTFLASH_ERROR_ADDR        0x01
#define INTFLASH_ERROR_SIZE        0x02
#define INTFLASH_ERROR_UNALIGNED   0x03
#define INTFLASH_ERROR_WRITE       0x04
#define INTFLASH_ERROR_ERASE       0x05
#define INTFLASH_ERROR_LOCK        0x06
#define INTFLASH_ERROR_TIMEOUT     0x07

/* 写保护状态 */
#define INTFLASH_WRITE_ENABLE      1
#define INTFLASH_WRITE_DISABLE     0

/* ============================================================================
 * Flash操作函数
 * ============================================================================ */

/**
 * @brief 初始化Flash（解锁Flash）
 * @return HAL状态
 */
HAL_StatusTypeDef INTFLASH_Init(void);

/**
 * @brief 去初始化Flash（锁定Flash）
 */
void INTFLASH_DeInit(void);

/**
 * @brief 擦除指定页
 * @param page_num 页号 (0-63)
 * @return 错误码
 */
uint8_t INTFLASH_ErasePage(uint32_t page_num);

/**
 * @brief 擦除指定地址所在页
 * @param address Flash地址
 * @return 错误码
 */
uint8_t INTFLASH_ErasePageByAddr(uint32_t address);

/**
 * @brief 擦除多个连续页
 * @param start_page 起始页号
 * @param page_count 页数
 * @return 错误码
 */
uint8_t INTFLASH_ErasePages(uint32_t start_page, uint32_t page_count);

/**
 * @brief 擦除用户数据区域（最后4KB）
 * @return 错误码
 */
uint8_t INTFLASH_EraseUserArea(void);

/**
 * @brief 写入32位数据
 * @param address Flash地址（必须是4字节对齐）
 * @param data 32位数据
 * @return 错误码
 */
uint8_t INTFLASH_WriteWord(uint32_t address, uint32_t data);

/**
 * @brief 写入半字（16位）数据
 * @param address Flash地址（必须是2字节对齐）
 * @param data 16位数据
 * @return 错误码
 */
uint8_t INTFLASH_WriteHalfWord(uint32_t address, uint16_t data);

/**
 * @brief 写入字节数据（实际写入半字，高8位无效）
 * @param address Flash地址
 * @param data 8位数据
 * @return 错误码
 */
uint8_t INTFLASH_WriteByte(uint32_t address, uint8_t data);

/**
 * @brief 写入数据缓冲区（自动处理对齐）
 * @param address Flash起始地址
 * @param data 数据缓冲区指针
 * @param size 数据大小（字节）
 * @return 错误码
 */
uint8_t INTFLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief 从Flash读取数据
 * @param address Flash起始地址
 * @param buffer 读取缓冲区
 * @param size 读取大小（字节）
 * @return 错误码
 */
uint8_t INTFLASH_ReadBuffer(uint32_t address, uint8_t *buffer, uint32_t size);

/**
 * @brief 读取32位数据
 * @param address Flash地址
 * @return 32位数据
 */
uint32_t INTFLASH_ReadWord(uint32_t address);

/**
 * @brief 读取半字（16位）数据
 * @param address Flash地址
 * @return 16位数据
 */
uint16_t INTFLASH_ReadHalfWord(uint32_t address);

/**
 * @brief 读取字节数据
 * @param address Flash地址
 * @return 8位数据
 */
uint8_t INTFLASH_ReadByte(uint32_t address);

/* ============================================================================
 * Flash保护和管理函数
 * ============================================================================ */

/**
 * @brief 获取地址所在的页号
 * @param address Flash地址
 * @return 页号(0-63)，无效地址返回0xFFFFFFFF
 */
uint32_t INTFLASH_GetPageNumber(uint32_t address);

/**
 * @brief 获取页起始地址
 * @param page_num 页号
 * @return 页起始地址
 */
uint32_t INTFLASH_GetPageAddress(uint32_t page_num);

/**
 * @brief 验证Flash数据完整性（比较数据）
 * @param address Flash地址
 * @param data 待比较数据
 * @param size 数据大小
 * @return 1:匹配, 0:不匹配
 */
uint8_t INTFLASH_VerifyData(uint32_t address, const uint8_t *data, uint32_t size);

/**
 * @brief 获取Flash剩余可用空间（字节）
 * @return 可用空间大小
 */
uint32_t INTFLASH_GetFreeSpace(void);

/**
 * @brief 检查Flash是否为空（全0xFF）
 * @param address 起始地址
 * @param size 检查大小
 * @return 1:全空, 0:非空
 */
uint8_t INTFLASH_IsEmpty(uint32_t address, uint32_t size);

/**
 * @brief 检查地址是否有效
 * @param address Flash地址
 * @return 1:有效, 0:无效
 */
uint8_t INTFLASH_IsAddressValid(uint32_t address);

/**
 * @brief 检查地址是否在用户区域内
 * @param address Flash地址
 * @return 1:在用户区域, 0:不在
 */
uint8_t INTFLASH_IsUserAreaAddress(uint32_t address);

#endif /* INTERNALFLASH_H */