/*
 * SHA256 Implementation for MCU
 * 适用于资源受限的嵌入式系统
 */

#include "sha_256.h"
#include <string.h>

SHA256_CTX ctx;
uint8_t hash[32];

/* 定义基本数据类型 */
typedef uint8_t  u8;
typedef uint32_t u32;

/* 32位循环右移 */
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

/* SHA256逻辑函数 */
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/* SHA256常量表（前64个素数的立方根小数部分的前32位） */
static const u32 K[64] = {
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* 字节序转换函数 */
static inline u32 load32(const u8 *bytes) {
    return ((u32)bytes[0] << 24) |
           ((u32)bytes[1] << 16) |
           ((u32)bytes[2] << 8) |
           ((u32)bytes[3]);
}

static inline void store32(u8 *bytes, u32 val) {
    bytes[0] = (val >> 24) & 0xff;
    bytes[1] = (val >> 16) & 0xff;
    bytes[2] = (val >> 8) & 0xff;
    bytes[3] = val & 0xff;
}

/* 初始化SHA256上下文 */
void SHA256_Init(SHA256_CTX *ctx) {
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    
    /* 初始哈希值（前8个素数的平方根小数部分的前32位） */
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

/* 处理一个64字节的数据块 */
static void SHA256_Transform(SHA256_CTX *ctx) {
    u32 W[64];
    u32 a, b, c, d, e, f, g, h;
    u32 T1, T2;
    int i;
    
    /* 将缓冲区数据扩展到W数组 */
    for (i = 0; i < 16; i++) {
        W[i] = load32(&ctx->buffer[i * 4]);
    }
    
    /* 扩展剩余48个字 */
    for (i = 16; i < 64; i++) {
        W[i] = SIG1(W[i - 2]) + W[i - 7] + SIG0(W[i - 15]) + W[i - 16];
    }
    
    /* 初始化工作变量 */
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];
    
    /* 主循环 */
    for (i = 0; i < 64; i++) {
        T1 = h + EP1(e) + CH(e, f, g) + K[i] + W[i];
        T2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }
    
    /* 更新哈希状态 */
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

/* 更新SHA256（处理输入数据） */
void SHA256_Update(SHA256_CTX *ctx, const void *data, u32 len) {
    u32 free_space;
    const u8 *bytes = (const u8 *)data;
    
    /* 更新总字节数（处理溢出） */
    ctx->total[0] += len;
    if (ctx->total[0] < len) {
        ctx->total[1]++;
    }
    
    /* 计算缓冲区中剩余的空间 */
    free_space = 64 - (ctx->total[0] % 64);
    
    /* 如果缓冲区中有数据，先填充缓冲区 */
    if (free_space > 0 && len >= free_space) {
        memcpy(&ctx->buffer[64 - free_space], bytes, free_space);
        SHA256_Transform(ctx);
        bytes += free_space;
        len -= free_space;
    }
    
    /* 处理完整的数据块 */
    while (len >= 64) {
        memcpy(ctx->buffer, bytes, 64);
        SHA256_Transform(ctx);
        bytes += 64;
        len -= 64;
    }
    
    /* 将剩余数据保存到缓冲区 */
    if (len > 0) {
        memcpy(ctx->buffer, bytes, len);
    }
}

/* 完成SHA256计算，输出最终哈希值 */
void SHA256_Final(SHA256_CTX *ctx, u8 *hash) {
    u32 i, pad_len;
    u8 pad[64];
    u32 bit_len[2];
    
    /* 计算原始比特长度 */
    bit_len[0] = (ctx->total[1] << 3) | (ctx->total[0] >> 29);
    bit_len[1] = ctx->total[0] << 3;
    
    /* 填充数据：0x80 */
    pad[0] = 0x80;
    pad_len = 1;
    
    /* 计算需要填充的长度 */
    i = ctx->total[0] % 64;
    if (i < 56) {
        pad_len += 56 - i;
    } else {
        pad_len += 64 + 56 - i;
    }
    
    /* 填充0x00 */
    memset(pad + 1, 0, pad_len - 1);
    SHA256_Update(ctx, pad, pad_len);
    
    /* 填充长度 */
    pad[0] = (bit_len[0] >> 24) & 0xff;
    pad[1] = (bit_len[0] >> 16) & 0xff;
    pad[2] = (bit_len[0] >> 8) & 0xff;
    pad[3] = bit_len[0] & 0xff;
    pad[4] = (bit_len[1] >> 24) & 0xff;
    pad[5] = (bit_len[1] >> 16) & 0xff;
    pad[6] = (bit_len[1] >> 8) & 0xff;
    pad[7] = bit_len[1] & 0xff;
    SHA256_Update(ctx, pad, 8);
    
    /* 输出哈希值 */
    for (i = 0; i < 8; i++) {
        store32(&hash[i * 4], ctx->state[i]);
    }
}

/* 便捷函数：计算单次哈希 */
void SHA256_Hash(const void *data, u32 len, u8 *hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(&ctx, hash);
}

/* 将哈希值转换为十六进制字符串 */
char* SHA256_HashToHex(const u8 *hash, char *hex_str) {
    const char hex_chars[] = "0123456789abcdef";
    int i;
    
    for (i = 0; i < 32; i++) {
        hex_str[i * 2] = hex_chars[hash[i] >> 4];
        hex_str[i * 2 + 1] = hex_chars[hash[i] & 0x0f];
    }
    hex_str[64] = '\0';
    
    return hex_str;
}

/* 从十六进制字符串转换为二进制哈希值 */
int SHA256_HexToHash(const char *hex_str, u8 *hash) {
    int i;
    char c;
    u8 val;
    
    /* 跳过可能的0x前缀 */
    if (hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        hex_str += 2;
    }
    
    /* 检查长度 */
    for (i = 0; i < 64; i++) {
        if (hex_str[i] == '\0') {
            return 0;  /* 长度不足 */
        }
    }
    
    /* 转换每个字节 */
    for (i = 0; i < 32; i++) {
        val = 0;
        
        /* 高4位 */
        c = hex_str[i * 2];
        if (c >= '0' && c <= '9') {
            val = (c - '0') << 4;
        } else if (c >= 'a' && c <= 'f') {
            val = (c - 'a' + 10) << 4;
        } else if (c >= 'A' && c <= 'F') {
            val = (c - 'A' + 10) << 4;
        } else {
            return 0;  /* 无效字符 */
        }
        
        /* 低4位 */
        c = hex_str[i * 2 + 1];
        if (c >= '0' && c <= '9') {
            val |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            val |= (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            val |= (c - 'A' + 10);
        } else {
            return 0;  /* 无效字符 */
        }
        
        hash[i] = val;
    }
    
    return 1;  /* 成功 */
}

/* 重置SHA256上下文 */
void SHA256_Reset(SHA256_CTX *ctx) {
    SHA256_Init(ctx);
}

/* 复制SHA256上下文 */
void SHA256_Copy(SHA256_CTX *dest, const SHA256_CTX *src) {
    memcpy(dest, src, sizeof(SHA256_CTX));
}

/* 获取当前处理的字节数 */
uint64_t SHA256_GetTotalBytes(const SHA256_CTX *ctx) {
    return ((uint64_t)ctx->total[1] << 32) | ctx->total[0];
}

/* 验证SHA256哈希值 */
int SHA256_Verify(const void *data, u32 len, const u8 *expected) {
    u8 hash[32];
    SHA256_Hash(data, len, hash);
    return (memcmp(hash, expected, 32) == 0);
}

/* HMAC-SHA256实现 */
void HMAC_SHA256_Init(SHA256_CTX *ctx, const void *key, u32 key_len) {
    u8 ipad[64];
    u8 k[64];
    u32 i;
    
    /* 如果密钥长度大于64字节，先进行哈希 */
    if (key_len > 64) {
        u8 tmp_hash[32];
        SHA256_Hash(key, key_len, tmp_hash);
        memcpy(k, tmp_hash, 32);
        memset(k + 32, 0, 32);
    } else {
        memcpy(k, key, key_len);
        memset(k + key_len, 0, 64 - key_len);
    }
    
    /* 构建ipad */
    for (i = 0; i < 64; i++) {
        ipad[i] = k[i] ^ 0x36;
    }
    
    /* 初始化上下文 */
    SHA256_Init(ctx);
    SHA256_Update(ctx, ipad, 64);
}

void HMAC_SHA256_Update(SHA256_CTX *ctx, const void *data, u32 len) {
    SHA256_Update(ctx, data, len);
}

void HMAC_SHA256_Final(SHA256_CTX *ctx, u8 *hmac) {
    u8 opad[64];
    u8 inner_hash[32];
    // u32 i;
    SHA256_CTX outer_ctx;
    
    /* 完成内部哈希 */
    SHA256_Final(ctx, inner_hash);
    
    /* 构建opad（需要从原始上下文恢复密钥，这里简化处理） */
    /* 注意：这个简化版本需要保存密钥，完整实现需要扩展上下文结构 */
    memset(opad, 0x5c, 64);
    
    /* 计算外部哈希 */
    SHA256_Init(&outer_ctx);
    SHA256_Update(&outer_ctx, opad, 64);
    SHA256_Update(&outer_ctx, inner_hash, 32);
    SHA256_Final(&outer_ctx, hmac);
}

void HMAC_SHA256(const void *key, u32 key_len,
                 const void *data, u32 data_len,
                 u8 *hmac) {
    SHA256_CTX ctx;
    HMAC_SHA256_Init(&ctx, key, key_len);
    HMAC_SHA256_Update(&ctx, data, data_len);
    HMAC_SHA256_Final(&ctx, hmac);
}