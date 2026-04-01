#include "aes_ecb.h"
#include <stdio.h>

// 静态缓冲区 - 避免动态内存分配
static uint8_t g_input_buffer[MAX_INPUT_BYTE_LEN + AES_BLOCK_SIZE];  // 输入缓冲区（含填充空间）
static uint8_t g_output_buffer[MAX_INPUT_BYTE_LEN + AES_BLOCK_SIZE]; // 输出缓冲区
static uint8_t g_key_buffer[AES_KEY_LEN_256];                         // 密钥缓冲区
static uint32_t g_round_keys[60];                                     // 轮密钥缓冲区（AES-256需要60个）

// AES S盒
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// AES逆S盒
static const uint8_t inv_sbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// 轮常量
static const uint8_t rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// 十六进制字符转字节
static uint8_t hex_char_to_byte(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

// 十六进制字符串转字节数组
static int hex_string_to_bytes(const char *hex, uint8_t *bytes, int max_len) {
    int len = strlen(hex);
    if (len % 2 != 0 || len > max_len * 2) return 0;
    
    for (int i = 0; i < len / 2; i++) {
        bytes[i] = (hex_char_to_byte(hex[i * 2]) << 4) | hex_char_to_byte(hex[i * 2 + 1]);
    }
    return len / 2;
}

// 字节数组转十六进制字符串
static void bytes_to_hex_string(const uint8_t *bytes, int len, char *hex) {
    const char *hex_chars = "0123456789ABCDEF";
    for (int i = 0; i < len; i++) {
        hex[i * 2] = hex_chars[bytes[i] >> 4];
        hex[i * 2 + 1] = hex_chars[bytes[i] & 0x0F];
    }
    hex[len * 2] = '\0';
}

// 密钥扩展
static void key_expansion(const uint8_t *key, uint32_t *round_keys) {
    uint8_t temp[4];
    uint32_t *w = round_keys;
    
    // 前8个字直接复制密钥
    for (int i = 0; i < 8; i++) {
        w[i] = (key[4 * i] << 24) | (key[4 * i + 1] << 16) | 
               (key[4 * i + 2] << 8) | key[4 * i + 3];
    }
    
    // 生成剩余的字
    for (int i = 8; i < 60; i++) {
        uint32_t temp_word = w[i - 1];
        
        if (i % 8 == 0) {
            // 循环左移8位
            temp[0] = (temp_word >> 16) & 0xFF;
            temp[1] = (temp_word >> 8) & 0xFF;
            temp[2] = temp_word & 0xFF;
            temp[3] = (temp_word >> 24) & 0xFF;
            
            // S盒替换
            for (int j = 0; j < 4; j++) {
                temp[j] = sbox[temp[j]];
            }
            
            // 异或轮常量
            temp[0] ^= rcon[i / 8];
            
            temp_word = (temp[0] << 24) | (temp[1] << 16) | (temp[2] << 8) | temp[3];
        } else if (i % 8 == 4) {
            // 对于每8个字中的第4个字，应用S盒
            temp[0] = (temp_word >> 24) & 0xFF;
            temp[1] = (temp_word >> 16) & 0xFF;
            temp[2] = (temp_word >> 8) & 0xFF;
            temp[3] = temp_word & 0xFF;
            
            for (int j = 0; j < 4; j++) {
                temp[j] = sbox[temp[j]];
            }
            
            temp_word = (temp[0] << 24) | (temp[1] << 16) | (temp[2] << 8) | temp[3];
        }
        
        w[i] = w[i - 8] ^ temp_word;
    }
}

// 字节替换
static void sub_bytes(uint8_t *state) {
    for (int i = 0; i < 16; i++) {
        state[i] = sbox[state[i]];
    }
}

// 逆字节替换
static void inv_sub_bytes(uint8_t *state) {
    for (int i = 0; i < 16; i++) {
        state[i] = inv_sbox[state[i]];
    }
}

// 行移位
static void shift_rows(uint8_t *state) {
    uint8_t temp;
    
    // 第2行循环左移1字节
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;
    
    // 第3行循环左移2字节
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    // 第4行循环左移3字节
    temp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = temp;
}

// 逆行移位
static void inv_shift_rows(uint8_t *state) {
    uint8_t temp;
    
    // 第2行循环右移1字节
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;
    
    // 第3行循环右移2字节
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    // 第4行循环右移3字节
    temp = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = state[3];
    state[3] = temp;
}

// GF(2^8)乘法
static uint8_t gf_multiply(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) result ^= a;
        uint8_t high_bit = a & 0x80;
        a <<= 1;
        if (high_bit) a ^= 0x1B;
        b >>= 1;
    }
    return result;
}

// 列混合
static void mix_columns(uint8_t *state) {
    uint8_t temp[16];
    
    for (int i = 0; i < 4; i++) {
        int col = i * 4;
        temp[col] = gf_multiply(0x02, state[col]) ^ gf_multiply(0x03, state[col + 1]) ^ 
                    state[col + 2] ^ state[col + 3];
        temp[col + 1] = state[col] ^ gf_multiply(0x02, state[col + 1]) ^ 
                        gf_multiply(0x03, state[col + 2]) ^ state[col + 3];
        temp[col + 2] = state[col] ^ state[col + 1] ^ 
                        gf_multiply(0x02, state[col + 2]) ^ gf_multiply(0x03, state[col + 3]);
        temp[col + 3] = gf_multiply(0x03, state[col]) ^ state[col + 1] ^ 
                        state[col + 2] ^ gf_multiply(0x02, state[col + 3]);
    }
    
    memcpy(state, temp, 16);
}

// 逆列混合
static void inv_mix_columns(uint8_t *state) {
    uint8_t temp[16];
    
    for (int i = 0; i < 4; i++) {
        int col = i * 4;
        temp[col] = gf_multiply(0x0E, state[col]) ^ gf_multiply(0x0B, state[col + 1]) ^ 
                    gf_multiply(0x0D, state[col + 2]) ^ gf_multiply(0x09, state[col + 3]);
        temp[col + 1] = gf_multiply(0x09, state[col]) ^ gf_multiply(0x0E, state[col + 1]) ^ 
                        gf_multiply(0x0B, state[col + 2]) ^ gf_multiply(0x0D, state[col + 3]);
        temp[col + 2] = gf_multiply(0x0D, state[col]) ^ gf_multiply(0x09, state[col + 1]) ^ 
                        gf_multiply(0x0E, state[col + 2]) ^ gf_multiply(0x0B, state[col + 3]);
        temp[col + 3] = gf_multiply(0x0B, state[col]) ^ gf_multiply(0x0D, state[col + 1]) ^ 
                        gf_multiply(0x09, state[col + 2]) ^ gf_multiply(0x0E, state[col + 3]);
    }
    
    memcpy(state, temp, 16);
}

// 添加轮密钥
static void add_round_key(uint8_t *state, const uint32_t *round_keys, int round) {
    for (int i = 0; i < 4; i++) {
        uint32_t key = round_keys[round * 4 + i];
        state[i * 4] ^= (key >> 24) & 0xFF;
        state[i * 4 + 1] ^= (key >> 16) & 0xFF;
        state[i * 4 + 2] ^= (key >> 8) & 0xFF;
        state[i * 4 + 3] ^= key & 0xFF;
    }
}

// AES-256加密单个块
static void aes_encrypt_block(const uint8_t *input, uint8_t *output, const uint32_t *round_keys) {
    uint8_t state[16];
    memcpy(state, input, 16);
    
    // 初始轮
    add_round_key(state, round_keys, 0);
    
    // 主要轮
    for (int round = 1; round < 14; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, round_keys, round);
    }
    
    // 最终轮
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, round_keys, 14);
    
    memcpy(output, state, 16);
}

// AES-256解密单个块
static void aes_decrypt_block(const uint8_t *input, uint8_t *output, const uint32_t *round_keys) {
    uint8_t state[16];
    memcpy(state, input, 16);
    
    // 初始轮
    add_round_key(state, round_keys, 14);
    
    // 主要轮
    for (int round = 13; round > 0; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, round_keys, round);
        inv_mix_columns(state);
    }
    
    // 最终轮
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, round_keys, 0);
    
    memcpy(output, state, 16);
}

// ZeroPadding函数：将数据填充到指定块大小的倍数
static int zero_padding(uint8_t *data, int data_len, int block_size) {
    int padding_len = block_size - (data_len % block_size);
    if (padding_len == block_size) {
        padding_len = 0; // 如果已经是块大小的倍数，不填充
    }
    
    // 添加零填充
    for (int i = 0; i < padding_len; i++) {
        data[data_len + i] = 0;
    }
    
    return data_len + padding_len;
}

// 去除ZeroPadding
static int remove_zero_padding(uint8_t *data, int data_len) {
    // 从末尾开始去除零
    int i = data_len - 1;
    while (i >= 0 && data[i] == 0) {
        i--;
    }
    return i + 1;
}

// 加密接口实现（使用静态缓冲区）
int AES_ECB_Encrypt(const char *input, const char *key, char *output, int output_size) {
    if (!input || !key || !output) return AES_ERR_INPUT_NULL;
    
    // 转换密钥
    int key_len = hex_string_to_bytes(key, g_key_buffer, AES_KEY_LEN_256);
    if (key_len != AES_KEY_LEN_256) return AES_ERR_KEY_NULL;
    
    // 密钥扩展
    key_expansion(g_key_buffer, g_round_keys);
    
    // 转换输入数据
    int input_hex_len = strlen(input);
    if (input_hex_len > MAX_INPUT_HEX_LEN) return AES_ERR_INVALID_LEN;
    
    int input_byte_len = input_hex_len / 2;
    hex_string_to_bytes(input, g_input_buffer, input_byte_len);
    
    // 应用ZeroPadding
    int padded_len = zero_padding(g_input_buffer, input_byte_len, AES_BLOCK_SIZE);
    
    // 计算需要的输出缓冲区大小（十六进制格式）
    int required_output_size = padded_len * 2 + 1;
    if (output_size < required_output_size) return AES_ERR_BUFFER_TOO_SMALL;
    
    // 处理每个块
    int block_count = padded_len / AES_BLOCK_SIZE;
    uint8_t block_buffer[AES_BLOCK_SIZE];
    
    for (int i = 0; i < block_count; i++) {
        aes_encrypt_block(g_input_buffer + i * AES_BLOCK_SIZE, block_buffer, g_round_keys);
        bytes_to_hex_string(block_buffer, AES_BLOCK_SIZE, output + i * (AES_BLOCK_SIZE * 2));
    }
    
    return AES_SUCCESS;
}

// 解密接口实现（使用静态缓冲区）
int AES_ECB_Decrypt(const char *input, const char *key, char *output, int output_size) {
    if (!input || !key || !output) return AES_ERR_INPUT_NULL;
    
    // 转换密钥
    int key_len = hex_string_to_bytes(key, g_key_buffer, AES_KEY_LEN_256);
    if (key_len != AES_KEY_LEN_256) return AES_ERR_KEY_NULL;
    
    // 密钥扩展
    key_expansion(g_key_buffer, g_round_keys);
    
    // 输入密文长度（十六进制格式）
    int input_hex_len = strlen(input);
    if (input_hex_len % 32 != 0) return AES_ERR_INVALID_LEN;
    if (input_hex_len > MAX_INPUT_HEX_LEN) return AES_ERR_INVALID_LEN;
    
    int block_count = input_hex_len / 32;
    int decrypted_byte_len = block_count * AES_BLOCK_SIZE;
    
    if (decrypted_byte_len > MAX_INPUT_BYTE_LEN + AES_BLOCK_SIZE) return AES_ERR_INVALID_LEN;
    
    uint8_t input_block[AES_BLOCK_SIZE];
    uint8_t output_block[AES_BLOCK_SIZE];
    
    // 解密所有块
    for (int i = 0; i < block_count; i++) {
        hex_string_to_bytes(input + i * 32, input_block, AES_BLOCK_SIZE);
        aes_decrypt_block(input_block, output_block, g_round_keys);
        memcpy(g_output_buffer + i * AES_BLOCK_SIZE, output_block, AES_BLOCK_SIZE);
    }
    
    // 去除ZeroPadding
    int original_len = remove_zero_padding(g_output_buffer, decrypted_byte_len);
    
    // 检查输出缓冲区大小
    int required_output_size = original_len * 2 + 1;
    if (output_size < required_output_size) return AES_ERR_BUFFER_TOO_SMALL;
    
    // 转换为十六进制字符串
    bytes_to_hex_string(g_output_buffer, original_len, output);
    
    return AES_SUCCESS;
}