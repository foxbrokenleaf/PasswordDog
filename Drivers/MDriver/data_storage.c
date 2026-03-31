/* data_storage.c - 数据存储管理实现 */
#include "data_storage.h"
#include "oled_menu.h"
#include <string.h>
#include <stdio.h>

// 全局数据缓存
PlatformData g_platformData[50];
uint8_t g_platformCount = 0;

// 计算校验和
static uint32_t CalculateChecksum(uint8_t* data, uint16_t len) {
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// 初始化存储系统
bool DataStorage_Init(void) {
    StorageHeader header;
    
    // 先尝试读取头部
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    // 检查魔数是否有效
    if (header.magic == STORAGE_MAGIC_NUMBER && header.version == STORAGE_VERSION) {
        // 有效数据，加载所有平台
        return DataStorage_LoadAllPlatforms();
    } else {
        // 无效数据，格式化存储区域
        return DataStorage_Format();
    }
}

// 保存单个平台数据
bool DataStorage_SavePlatform(uint16_t index, const char* platform, const char* account, const char* password) {
    StoredPlatformData data;
    uint32_t addr;
    StorageHeader header;
    
    if (index >= 100) return false;
    
    // 准备数据
    memset(&data, 0xFF, sizeof(StoredPlatformData));
    strncpy(data.platform, platform, sizeof(data.platform) - 1);
    strncpy(data.account, account, sizeof(data.account) - 1);
    strncpy(data.password, password, sizeof(data.password) - 1);
    data.isValid = 0x5A;  // 有效标志
    
    // 计算存储地址（跳过头部，每个平台占256字节）
    addr = STORAGE_START_ADDR + sizeof(StorageHeader) + index * sizeof(StoredPlatformData);
    
    // 先擦除对应的扇区（如果需要）
    uint32_t sectorAddr = addr & ~(w25qxx.SectorSize - 1);
    if ((addr % w25qxx.SectorSize) == 0) {
        W25qxx_EraseSector(sectorAddr);
    }
    
    // 写入数据
    W25qxx_WriteBuffer((uint8_t*)&data, addr, sizeof(StoredPlatformData));
    
    // 更新头部信息
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    if (header.magic != STORAGE_MAGIC_NUMBER) {
        // 首次写入，初始化头部
        header.magic = STORAGE_MAGIC_NUMBER;
        header.version = STORAGE_VERSION;
        header.itemCount = 0;
    }
    
    // 更新数量
    if (index >= header.itemCount) {
        header.itemCount = index + 1;
    }
    
    // 计算校验和
    header.checksum = CalculateChecksum((uint8_t*)&header, sizeof(StorageHeader) - 4);
    
    // 写入头部
    W25qxx_EraseSector(STORAGE_START_ADDR);
    W25qxx_WriteBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    return true;
}

// 加载单个平台数据
bool DataStorage_LoadPlatform(uint16_t index, char* platform, char* account, char* password, uint8_t* isValid) {
    StoredPlatformData data;
    uint32_t addr;
    StorageHeader header;
    
    // 读取头部验证
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    if (header.magic != STORAGE_MAGIC_NUMBER || index >= header.itemCount) {
        return false;
    }
    
    // 计算存储地址
    addr = STORAGE_START_ADDR + sizeof(StorageHeader) + index * sizeof(StoredPlatformData);
    
    // 读取数据
    W25qxx_ReadBuffer((uint8_t*)&data, addr, sizeof(StoredPlatformData));
    
    // 复制数据
    if (platform) strcpy(platform, data.platform);
    if (account) strcpy(account, data.account);
    if (password) strcpy(password, data.password);
    if (isValid) *isValid = (data.isValid == 0x5A) ? 1 : 0;
    
    return (data.isValid == 0x5A);
}

// 加载所有平台数据到内存
bool DataStorage_LoadAllPlatforms(void) {
    StorageHeader header;
    StoredPlatformData data;
    uint32_t addr;
    
    // 读取头部
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    if (header.magic != STORAGE_MAGIC_NUMBER) {
        g_platformCount = 0;
        return false;
    }
    
    // 验证校验和
    uint32_t calcChecksum = CalculateChecksum((uint8_t*)&header, sizeof(StorageHeader) - 4);
    if (calcChecksum != header.checksum) {
        g_platformCount = 0;
        return false;
    }
    
    // 加载所有平台
    g_platformCount = 0;
    for (uint16_t i = 0; i < header.itemCount && i < 50; i++) {
        addr = STORAGE_START_ADDR + sizeof(StorageHeader) + i * sizeof(StoredPlatformData);
        W25qxx_ReadBuffer((uint8_t*)&data, addr, sizeof(StoredPlatformData));
        
        if (data.isValid == 0x5A) {
            strcpy(g_platformData[g_platformCount].platform, data.platform);
            strcpy(g_platformData[g_platformCount].account, data.account);
            strcpy(g_platformData[g_platformCount].password, data.password);
            g_platformData[g_platformCount].isValid = 1;
            g_platformCount++;
        }
    }
    
    return true;
}

// 删除平台数据
bool DataStorage_DeletePlatform(uint16_t index) {
    StoredPlatformData data;
    uint32_t addr;
    StorageHeader header;
    
    // 读取头部
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    if (header.magic != STORAGE_MAGIC_NUMBER || index >= header.itemCount) {
        return false;
    }
    
    // 标记为无效
    addr = STORAGE_START_ADDR + sizeof(StorageHeader) + index * sizeof(StoredPlatformData);
    memset(&data, 0xFF, sizeof(StoredPlatformData));
    data.isValid = 0x00;  // 标记无效
    
    W25qxx_WriteBuffer((uint8_t*)&data, addr, sizeof(StoredPlatformData));
    
    return true;
}

// 获取存储的平台数量
uint16_t DataStorage_GetPlatformCount(void) {
    StorageHeader header;
    W25qxx_ReadBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    if (header.magic == STORAGE_MAGIC_NUMBER) {
        return header.itemCount;
    }
    return 0;
}

// 格式化存储区域
bool DataStorage_Format(void) {
    StorageHeader header;
    
    // 初始化头部
    memset(&header, 0xFF, sizeof(header));
    header.magic = STORAGE_MAGIC_NUMBER;
    header.version = STORAGE_VERSION;
    header.itemCount = 0;
    header.checksum = CalculateChecksum((uint8_t*)&header, sizeof(StorageHeader) - 4);
    
    // 擦除并写入头部
    W25qxx_EraseSector(STORAGE_START_ADDR);
    W25qxx_WriteBuffer((uint8_t*)&header, STORAGE_START_ADDR, sizeof(StorageHeader));
    
    g_platformCount = 0;
    memset(g_platformData, 0, sizeof(g_platformData));
    
    return true;
}

// 添加新平台（辅助函数）
bool DataStorage_AddPlatform(const char* platform, const char* account, const char* password) {
    uint16_t count = DataStorage_GetPlatformCount();
    if (count >= 50) return false;
    
    if (DataStorage_SavePlatform(count, platform, account, password)) {
        // 更新内存缓存
        strcpy(g_platformData[g_platformCount].platform, platform);
        strcpy(g_platformData[g_platformCount].account, account);
        strcpy(g_platformData[g_platformCount].password, password);
        g_platformData[g_platformCount].isValid = 1;
        g_platformCount++;
        return true;
    }
    return false;
}