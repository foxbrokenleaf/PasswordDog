#ifndef __AES_ECB_H
#define __AES_ECB_H

#include <stdint.h>
#include <string.h>

// AES-256 密钥长度: 32字节 (256位)
#define AES_KEY_LEN_256 32

// AES 块大小: 16字节 (128位)
#define AES_BLOCK_SIZE 16

// 最大支持的输入数据长度（十六进制字符串长度）
// 可根据需要调整，这里设置为512字节（256字节原始数据）
#define MAX_INPUT_HEX_LEN 512
#define MAX_INPUT_BYTE_LEN (MAX_INPUT_HEX_LEN / 2)

// 错误码定义
#define AES_SUCCESS 0
#define AES_ERR_KEY_NULL 1
#define AES_ERR_INPUT_NULL 2
#define AES_ERR_INVALID_LEN 3
#define AES_ERR_BUFFER_TOO_SMALL 4

/**
 * @brief AES-256-ECB 加密接口（ZeroPadding，使用静态缓冲区）
 * @param input: 输入字符串（十六进制格式）
 * @param key: 密钥字符串（十六进制格式，64个字符表示32字节）
 * @param output: 输出缓冲区（需要足够空间，输出为十六进制字符串）
 * @param output_size: 输出缓冲区大小
 * @return 0: 成功, 其他: 失败
 */
int AES_ECB_Encrypt(const char *input, const char *key, char *output, int output_size);

/**
 * @brief AES-256-ECB 解密接口（ZeroPadding，使用静态缓冲区）
 * @param input: 输入字符串（十六进制格式）
 * @param key: 密钥字符串（十六进制格式，64个字符表示32字节）
 * @param output: 输出缓冲区（需要足够空间，输出为十六进制字符串）
 * @param output_size: 输出缓冲区大小
 * @return 0: 成功, 其他: 失败
 */
int AES_ECB_Decrypt(const char *input, const char *key, char *output, int output_size);

#endif /* __AES_ECB_H */