/* data_storage.c - W25Q256 数据存储实现 */
#include "data_storage.h"
#include <string.h>
#include <stdio.h>

// 存储头部地址
#define HEADER_ADDR             STORAGE_START_ADDR
#define FIRST_RECORD_ADDR       (STORAGE_START_ADDR + sizeof(StorageHeader))

// 静态变量
static StorageHeader g_header;
static bool g_headerLoaded = false;

// 计算校验和
static uint32_t CalculateChecksum(uint8_t* data, uint16_t len) {
    uint32_t sum = 0;
    uint16_t i;
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// 计算记录的校验字节
static uint8_t CalculateRecordCheckByte(PlatformRecord* record) {
    uint8_t sum = 0;
    uint8_t* p = (uint8_t*)record;
    uint16_t i;
    
    for (i = 0; i < sizeof(PlatformRecord) - 1; i++) {
        sum += p[i];
    }
    return ~sum + 1;  // 补码
}

// 验证记录
static bool IsRecordValid(PlatformRecord* record) {
    uint8_t checkByte = CalculateRecordCheckByte(record);
    return (checkByte == 0);  // 有效记录所有字节和应为0
}

// 获取记录存储地址
static uint32_t GetRecordAddress(uint32_t recordNo) {
    return FIRST_RECORD_ADDR + (recordNo - 1) * RECORD_SIZE;
}

// 加载头部
static bool LoadHeader(void) {
    if (g_headerLoaded) return true;
    
    W25qxx_ReadBuffer((uint8_t*)&g_header, HEADER_ADDR, sizeof(StorageHeader));
    
    if (g_header.magic == STORAGE_MAGIC_NUMBER && g_header.version == STORAGE_VERSION) {
        // 验证校验和
        uint32_t calcChecksum = CalculateChecksum((uint8_t*)&g_header, sizeof(StorageHeader) - 4);
        if (calcChecksum == g_header.checksum) {
            g_headerLoaded = true;
            return true;
        }
    }
    
    return false;
}

// 保存头部
static bool SaveHeader(void) {
    g_header.checksum = CalculateChecksum((uint8_t*)&g_header, sizeof(StorageHeader) - 4);
    
    // 先擦除头部扇区
    uint32_t sectorAddr = HEADER_ADDR & ~(w25qxx.SectorSize - 1);
    W25qxx_EraseSector(sectorAddr);
    
    // 写入头部
    W25qxx_WriteBuffer((uint8_t*)&g_header, HEADER_ADDR, sizeof(StorageHeader));
    
    return true;
}

// 初始化存储系统
bool DataStorage_Init(void) {
    if (!LoadHeader()) {
        return DataStorage_Format();
    }
    return true;
}

// 格式化存储
bool DataStorage_Format(void) {
    // 初始化头部
    memset(&g_header, 0, sizeof(StorageHeader));
    g_header.magic = STORAGE_MAGIC_NUMBER;
    g_header.version = STORAGE_VERSION;
    g_header.maxRecords = MAX_RECORD_COUNT;
    g_header.usedRecords = 0;
    g_header.nextRecordNo = 1;
    
    // 保存头部
    if (!SaveHeader()) return false;
    
    g_headerLoaded = true;
    return true;
}

// 添加记录
uint32_t DataStorage_AddRecord(const char* platform, const char* account, const char* password) {
    PlatformRecord record;
    uint32_t recordNo;
    uint32_t addr;
    
    if (!g_headerLoaded) LoadHeader();
    
    // 检查是否还有空间
    if (g_header.usedRecords >= g_header.maxRecords) {
        return 0;
    }
    
    // 获取下一个可用的记录号
    recordNo = g_header.nextRecordNo;
    
    // 准备记录
    memset(&record, 0, sizeof(PlatformRecord));
    record.no = recordNo;
    strncpy(record.platform, platform, sizeof(record.platform) - 1);
    record.platform[sizeof(record.platform) - 1] = '\0';
    strncpy(record.account, account, sizeof(record.account) - 1);
    record.account[sizeof(record.account) - 1] = '\0';
    strncpy(record.password, password, sizeof(record.password) - 1);
    record.password[sizeof(record.password) - 1] = '\0';
    
    // 计算存储地址
    addr = GetRecordAddress(recordNo);
    
    // 擦除所在扇区（如果需要）
    uint32_t sectorAddr = addr & ~(w25qxx.SectorSize - 1);
    if ((addr % w25qxx.SectorSize) == 0) {
        W25qxx_EraseSector(sectorAddr);
    }
    
    // 写入记录
    W25qxx_WriteBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    // 更新头部
    g_header.usedRecords++;
    g_header.nextRecordNo++;
    
    // 查找下一个有效记录号（跳过可能被删除的）
    while (g_header.nextRecordNo <= g_header.maxRecords) {
        PlatformRecord testRecord;
        uint32_t testAddr = GetRecordAddress(g_header.nextRecordNo);
        W25qxx_ReadBuffer((uint8_t*)&testRecord, testAddr, sizeof(PlatformRecord));
        if (!IsRecordValid(&testRecord)) {
            break;
        }
        g_header.nextRecordNo++;
    }
    
    SaveHeader();
    
    return recordNo;
}

// 根据记录号获取记录
bool DataStorage_GetRecord(uint32_t recordNo, char* platform, char* account, char* password) {
    PlatformRecord record;
    uint32_t addr;
    
    if (recordNo < 1 || recordNo > g_header.maxRecords) {
        return false;
    }
    
    addr = GetRecordAddress(recordNo);
    W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    // 验证记录
    if (!IsRecordValid(&record)) {
        return false;
    }
    
    if (platform) strcpy(platform, record.platform);
    if (account) strcpy(account, record.account);
    if (password) strcpy(password, record.password);
    
    return true;
}

// 根据索引获取记录（用于遍历）
bool DataStorage_GetRecordByIndex(uint32_t index, char* platform, char* account, char* password, uint32_t* recordNo) {
    uint32_t found = 0;
    
    for (uint32_t i = 1; i <= g_header.maxRecords && found <= index; i++) {
        PlatformRecord record;
        uint32_t addr = GetRecordAddress(i);
        
        W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
        
        if (IsRecordValid(&record)) {
            if (found == index) {
                if (platform) strcpy(platform, record.platform);
                if (account) strcpy(account, record.account);
                if (password) strcpy(password, record.password);
                if (recordNo) *recordNo = record.no;
                return true;
            }
            found++;
        }
    }
    
    return false;
}

// 删除记录
bool DataStorage_DeleteRecord(uint32_t recordNo) {
    PlatformRecord record;
    uint32_t addr;
    
    if (!g_headerLoaded) LoadHeader();
    if (recordNo < 1 || recordNo > g_header.maxRecords) {
        return false;
    }
    
    addr = GetRecordAddress(recordNo);
    W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    if (!IsRecordValid(&record)) {
        return false;
    }
    
    // 清除记录（全部写0）
    memset(&record, 0, sizeof(PlatformRecord));
    W25qxx_WriteBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    // 更新头部
    g_header.usedRecords--;
    
    // 如果删除的记录号小于 nextRecordNo，更新 nextRecordNo
    if (recordNo < g_header.nextRecordNo) {
        g_header.nextRecordNo = recordNo;
    }
    
    SaveHeader();
    
    return true;
}

// 更新记录
bool DataStorage_UpdateRecord(uint32_t recordNo, const char* platform, const char* account, const char* password) {
    PlatformRecord record;
    uint32_t addr;
    
    if (recordNo < 1 || recordNo > g_header.maxRecords) {
        return false;
    }
    
    addr = GetRecordAddress(recordNo);
    
    // 读取现有记录
    W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    if (!IsRecordValid(&record)) {
        return false;
    }
    
    // 更新字段
    if (platform) {
        strncpy(record.platform, platform, sizeof(record.platform) - 1);
        record.platform[sizeof(record.platform) - 1] = '\0';
    }
    if (account) {
        strncpy(record.account, account, sizeof(record.account) - 1);
        record.account[sizeof(record.account) - 1] = '\0';
    }
    if (password) {
        strncpy(record.password, password, sizeof(record.password) - 1);
        record.password[sizeof(record.password) - 1] = '\0';
    }
    
    // 写回
    W25qxx_WriteBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
    
    return true;
}

// 获取总记录数（包括已删除的）
uint32_t DataStorage_GetTotalCount(void) {
    if (!g_headerLoaded) LoadHeader();
    return g_header.maxRecords;
}

// 获取有效记录数
uint32_t DataStorage_GetValidCount(void) {
    if (!g_headerLoaded) LoadHeader();
    return g_header.usedRecords;
}

// 根据平台名称查找
bool DataStorage_FindByPlatform(const char* platform, uint32_t* recordNo) {
    for (uint32_t i = 1; i <= g_header.maxRecords; i++) {
        PlatformRecord record;
        uint32_t addr = GetRecordAddress(i);
        
        W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
        
        if (IsRecordValid(&record) && strcmp(record.platform, platform) == 0) {
            if (recordNo) *recordNo = record.no;
            return true;
        }
    }
    
    return false;
}

// 获取所有平台名称
void DataStorage_GetAllPlatforms(char platforms[][128], uint32_t* count) {
    uint32_t idx = 0;
    
    for (uint32_t i = 1; i <= g_header.maxRecords && idx < *count; i++) {
        PlatformRecord record;
        uint32_t addr = GetRecordAddress(i);
        
        W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
        
        if (IsRecordValid(&record)) {
            strcpy(platforms[idx], record.platform);
            idx++;
        }
    }
    
    *count = idx;
}

// 碎片整理
bool DataStorage_Defragment(void) {
    uint32_t writePos = 1;
    PlatformRecord tempRecords[MAX_RECORD_COUNT];
    uint32_t validCount = 0;
    
    // 收集所有有效记录
    for (uint32_t i = 1; i <= g_header.maxRecords; i++) {
        PlatformRecord record;
        uint32_t addr = GetRecordAddress(i);
        
        W25qxx_ReadBuffer((uint8_t*)&record, addr, sizeof(PlatformRecord));
        
        if (IsRecordValid(&record)) {
            memcpy(&tempRecords[validCount], &record, sizeof(PlatformRecord));
            tempRecords[validCount].no = writePos;
            validCount++;
            writePos++;
        }
    }
    
    // 擦除数据区域
    for (uint32_t sector = 0; sector < (w25qxx.SectorCount); sector++) {
        W25qxx_EraseSector(STORAGE_START_ADDR + sector * w25qxx.SectorSize);
    }
    
    // 重新写入头部
    SaveHeader();
    
    // 重新写入记录
    for (uint32_t i = 0; i < validCount; i++) {
        uint32_t addr = GetRecordAddress(i + 1);
        W25qxx_WriteBuffer((uint8_t*)&tempRecords[i], addr, sizeof(PlatformRecord));
    }
    
    // 更新头部
    g_header.usedRecords = validCount;
    g_header.nextRecordNo = validCount + 1;
    SaveHeader();
    
    return true;
}

// 获取剩余空间（可存储的记录数）
uint32_t DataStorage_GetFreeSpace(void) {
    if (!g_headerLoaded) LoadHeader();
    return g_header.maxRecords - g_header.usedRecords;
}