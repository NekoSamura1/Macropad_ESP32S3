#pragma once
#include "mbedtls/aes.h"
#include <Arduino.h>
#include <SPIFFS.h>

#define FILE_PATH "/records.bin"
#define TEMP_FILE_PATH "/temp_records.bin"

//?##################################################################################
//*         AES
#define BUF_SIZE 1024
#define KEY_SIZE 32 // AES-256
#define IV_SIZE 16  // 128 бит для CBC

typedef struct {
    uint16_t data_len;
    uint8_t iv[IV_SIZE];
} RecordHeader;

static const uint8_t aes_key[KEY_SIZE] = {0xC3, 0x20, 0x2D, 0xC8, 0x48, 0x4E, 0x04, 0x2B, 0x8A, 0xA3, 0xEB, 0x8A, 0x6C, 0xFE, 0xF9, 0x30, 0x98, 0xFD, 0xCC, 0xA3, 0xC6, 0xC3, 0xB7, 0x1E, 0x1A, 0xAB, 0x34, 0xED, 0x82, 0x34, 0x9B, 0xB9};

// size_t remove_pkcs7_padding(uint8_t *data, size_t len);
// void add_pkcs7_padding(uint8_t* data, size_t len);
// inline uint_fast16_t calculate_padded_length(int_fast16_t len);

void appendRecordRaw(const uint8_t* iv, const uint8_t* data, uint16_t data_len);
void deleteRecord(uint32_t n);
void deleteFile(const char* fileName);

/// @brief
/// @param n
/// @param data should be nullptr, allocates memory
/// @param shoud not be nullptr
void readRecordRaw(uint32_t n, uint8_t* data, RecordHeader* constheader);

void testEncryptDecrypt(const char* text);

void appendEncryptedRecord(const char* text);
void readEncryptedRecord(size_t number, char* text);
