/* data_storage.h - W25Q256 数据存储管理 */
#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "w25q256.h"

// W25Q256: 256Mbit = 32MByte = 33,554,432 字节
#define W25Q256_TOTAL_SIZE      33554432UL

// 单条记录结构定义
#define FIELD_NO_LEN            2       // 序号字段长度
#define FIELD_PLATFORM_LEN      128     // 平台字段长度
#define FIELD_ACCOUNT_LEN       128     // 账号字段长度
#define FIELD_PASSWORD_LEN      256     // 密码字段长度
#define RECORD_SIZE             (FIELD_NO_LEN + FIELD_PLATFORM_LEN + FIELD_ACCOUNT_LEN + FIELD_PASSWORD_LEN)  // 514字节

// 存储容量计算
#define MAX_RECORD_COUNT        (W25Q256_TOTAL_SIZE / RECORD_SIZE)  // 33554432 / 514 = 65280条

// 存储区域定义
#define STORAGE_START_ADDR      0x00000000
#define STORAGE_MAGIC_NUMBER    0x5A5A5A5A
#define STORAGE_VERSION         0x0002

// 记录头部结构
typedef __packed struct {
    uint16_t no;                    // 记录序号
    uint8_t reserved[6];            // 保留字段
    uint8_t isValid;                // 有效性标志 (0x5A=有效, 0x00=无效)
    uint8_t checkByte;              // 校验字节
} RecordHeader;

// 完整记录结构
typedef __packed struct {
    uint16_t no;                    // 序号 (2字节)
    char platform[128];             // 平台 (128字节)
    char account[128];              // 账号 (128字节)
    char password[256];             // 密码 (256字节)
} PlatformRecord;

// 存储头部结构
typedef __packed struct {
    uint32_t magic;                 // 魔数
    uint16_t version;               // 版本号
    uint32_t maxRecords;            // 最大存储记录数
    uint32_t usedRecords;           // 已使用记录数
    uint32_t nextRecordNo;          // 下一个记录序号
    uint32_t checksum;              // 校验和
    uint8_t reserved[236];          // 保留字节，凑够256字节
} StorageHeader;

// API函数声明
bool DataStorage_Init(void);
bool DataStorage_Format(void);

// 记录操作
uint32_t DataStorage_AddRecord(const char* platform, const char* account, const char* password);
bool DataStorage_GetRecord(uint32_t recordNo, char* platform, char* account, char* password);
bool DataStorage_GetRecordByIndex(uint32_t index, char* platform, char* account, char* password, uint32_t* recordNo);
bool DataStorage_DeleteRecord(uint32_t recordNo);
bool DataStorage_UpdateRecord(uint32_t recordNo, const char* platform, const char* account, const char* password);

// 查询功能
uint32_t DataStorage_GetTotalCount(void);
uint32_t DataStorage_GetValidCount(void);
bool DataStorage_FindByPlatform(const char* platform, uint32_t* recordNo);
void DataStorage_GetAllPlatforms(char platforms[][128], uint32_t* count);

// 维护功能
bool DataStorage_Defragment(void);  // 碎片整理
uint32_t DataStorage_GetFreeSpace(void);

#endif