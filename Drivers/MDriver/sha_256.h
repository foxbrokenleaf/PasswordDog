/*
 * sha256.h - SHA256 Algorithm for MCU
 * 适用于资源受限的嵌入式系统
 * 
 *     
 * 
 * 
 * 使用方法:
 *   SHA256_CTX ctx;
 *   uint8_t hash[32];
 *   
 *   SHA256_Init(&ctx);
 *   SHA256_Update(&ctx, data, len);
 *   SHA256_Final(&ctx, hash);
 */

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 类型定义
 * ============================================================================ */

/* SHA256上下文结构 - 用户不应直接访问内部成员 */
typedef struct {
    uint32_t total[2];       /* 已处理的字节数 (64位计数器) */
    uint32_t state[8];       /* 哈希状态 */
    uint8_t  buffer[64];     /* 数据缓冲区 */
} SHA256_CTX;

/* SHA256哈希值长度 */
#define SHA256_HASH_SIZE    32   /* 256位 = 32字节 */
#define SHA256_BLOCK_SIZE   64   /* 512位 = 64字节 */

extern SHA256_CTX ctx;
extern uint8_t hash[32];

/* ============================================================================
 * 核心API函数
 * ============================================================================ */

/**
 * @brief 初始化SHA256上下文
 * @param ctx SHA256上下文指针
 */
void SHA256_Init(SHA256_CTX *ctx);

/**
 * @brief 添加数据到SHA256计算
 * @param ctx   SHA256上下文指针
 * @param data  输入数据指针
 * @param len   数据长度（字节）
 * @note 可多次调用以处理分块数据
 */
void SHA256_Update(SHA256_CTX *ctx, const void *data, uint32_t len);

/**
 * @brief 完成SHA256计算，输出最终哈希值
 * @param ctx  SHA256上下文指针
 * @param hash 输出缓冲区（至少32字节）
 * @note 调用后上下文将被清零，可重新初始化使用
 */
void SHA256_Final(SHA256_CTX *ctx, uint8_t *hash);

/* ============================================================================
 * 便捷函数
 * ============================================================================ */

/**
 * @brief 一次性计算数据的SHA256哈希值
 * @param data  输入数据指针
 * @param len   数据长度（字节）
 * @param hash  输出缓冲区（至少32字节）
 * @note 适用于小数据块的一步计算
 */
void SHA256_Hash(const void *data, uint32_t len, uint8_t *hash);

/**
 * @brief 将二进制哈希值转换为十六进制字符串
 * @param hash    32字节二进制哈希值
 * @param hex_str 输出字符串缓冲区（至少65字节，包含终止符）
 * @return 返回hex_str指针
 * @note 输出字符串格式：小写十六进制，无前缀，自动添加'\0'
 */
char* SHA256_HashToHex(const uint8_t *hash, char *hex_str);

/**
 * @brief 从十六进制字符串转换为二进制哈希值
 * @param hex_str 十六进制字符串（64个字符，可含或不含0x前缀）
 * @param hash    输出缓冲区（至少32字节）
 * @return 成功返回1，失败返回0
 * @note 支持大小写十六进制，自动跳过空白字符和0x前缀
 */
int SHA256_HexToHash(const char *hex_str, uint8_t *hash);

/* ============================================================================
 * 扩展功能
 * ============================================================================ */

/**
 * @brief 重置SHA256上下文（不清零，可重新使用）
 * @param ctx SHA256上下文指针
 */
void SHA256_Reset(SHA256_CTX *ctx);

/**
 * @brief 复制SHA256上下文
 * @param dest 目标上下文
 * @param src  源上下文
 */
void SHA256_Copy(SHA256_CTX *dest, const SHA256_CTX *src);

/**
 * @brief 获取当前处理的字节数
 * @param ctx SHA256上下文指针
 * @return 已处理的字节数（64位）
 * @note 返回值为64位，但MCU可能不支持，这里返回低32位
 */
uint64_t SHA256_GetTotalBytes(const SHA256_CTX *ctx);

/**
 * @brief 验证SHA256哈希值
 * @param data     原始数据指针
 * @param len      数据长度
 * @param expected 期望的哈希值（32字节）
 * @return 验证成功返回1，失败返回0
 */
int SHA256_Verify(const void *data, uint32_t len, const uint8_t *expected);

/**
 * @brief HMAC-SHA256初始化
 * @param ctx   SHA256上下文指针
 * @param key   密钥
 * @param key_len 密钥长度
 */
void HMAC_SHA256_Init(SHA256_CTX *ctx, const void *key, uint32_t key_len);

/**
 * @brief HMAC-SHA256更新
 * @param ctx   SHA256上下文指针
 * @param data  数据指针
 * @param len   数据长度
 */
void HMAC_SHA256_Update(SHA256_CTX *ctx, const void *data, uint32_t len);

/**
 * @brief HMAC-SHA256完成计算
 * @param ctx   SHA256上下文指针
 * @param hmac  输出HMAC值（32字节）
 */
void HMAC_SHA256_Final(SHA256_CTX *ctx, uint8_t *hmac);

/**
 * @brief 一次性计算HMAC-SHA256
 * @param key      密钥
 * @param key_len  密钥长度
 * @param data     数据指针
 * @param data_len 数据长度
 * @param hmac     输出HMAC值（32字节）
 */
void HMAC_SHA256(const void *key, uint32_t key_len,
                 const void *data, uint32_t data_len,
                 uint8_t *hmac);

#ifdef __cplusplus
}
#endif

#endif /* SHA256_H */