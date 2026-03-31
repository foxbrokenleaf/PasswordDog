/* data_storage.h - 数据存储管理头文件 */
#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "w25q256.h"
#include "oled_menu.h"

// 存储区域定义
#define STORAGE_START_ADDR      0x00000000  // 存储起始地址
#define STORAGE_MAGIC_NUMBER    0x5A5A5A5A  // 魔数，用于验证数据有效性
#define STORAGE_VERSION         0x0001      // 存储格式版本

// 单个平台数据存储结构（对齐到256字节边界，便于页写入）
#pragma pack(1)
typedef struct {
    char platform[32];      // 平台名称
    char account[64];       // 账号
    char password[65];      // 密码（SHA256字符串，64字符+结束符）
    uint8_t isValid;        // 是否有效 (0x5A表示有效)
    uint8_t reserved[94];   // 保留字节，凑够256字节
} StoredPlatformData;
#pragma pack()

// 存储头部结构
#pragma pack(1)
typedef struct {
    uint32_t magic;         // 魔数，验证数据有效性
    uint16_t version;       // 版本号
    uint16_t itemCount;     // 存储的平台数量
    uint32_t checksum;      // 校验和
    uint8_t reserved[240];  // 保留字节
} StorageHeader;
#pragma pack()

// API函数声明
bool DataStorage_Init(void);
bool DataStorage_SavePlatform(uint16_t index, const char* platform, const char* account, const char* password);
bool DataStorage_LoadPlatform(uint16_t index, char* platform, char* account, char* password, uint8_t* isValid);
bool DataStorage_LoadAllPlatforms(void);
bool DataStorage_DeletePlatform(uint16_t index);
uint16_t DataStorage_GetPlatformCount(void);
bool DataStorage_Format(void);

// 外部全局变量声明
extern PlatformData g_platformData[50];
extern uint8_t g_platformCount;

#endif