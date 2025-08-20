#pragma once
#include "mbedtls/aes.h"
#include <Arduino.h>
#include <SPIFFS.h>

#define BUF_SIZE 1024
#define KEY_SIZE 32 // AES-256
#define IV_SIZE 16  // 128 бит для CBC

typedef struct {
    uint16_t data_len;
    uint8_t iv[IV_SIZE];
} RecordHeader;


void appendRecordRaw(const uint8_t* iv, const uint8_t* data, uint16_t data_len);

/// @brief
/// @param n record number
/// @param data should be nullptr, allocates memory
/// @param header should not be nullptr
void readRecordRaw(uint32_t n, uint8_t** data, RecordHeader* const header);

void appendEncryptedRecord(const char* text);

/// @brief 
/// @param number record number
/// @param text should be nullptr, allocates memory
void readEncryptedRecord(size_t number, char* text);

void deleteRecord(uint32_t n);
void deleteFile(const char* fileName);