#include <stdint.h>
#include <Arduino.h>
#include "mbedtls/aes.h"
#include "config.h"

//?##################################################################################
//*         AES
#define BUF_SIZE 1024
#define KEY_SIZE 32 // AES-256
#define IV_SIZE 16 // 128 бит для CBC


typedef struct {
    uint8_t iv[IV_SIZE];
    uint16_t data_len;
} RecordHeader;

void add_record(const char* text);


size_t remove_pkcs7_padding(uint8_t *data, size_t len);
void add_pkcs7_padding(uint8_t* data, size_t len);
int_fast16_t calculate_padded_length(int_fast16_t len);


#pragma once