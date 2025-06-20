#include "CryptoText.h"
//?##################################################################################
//*         TEXT functions

void add_record(const char* text) {
    size_t text_len = strlen(text);
    size_t padded_len = calculate_padded_length(text_len);

    // Подготовка данных
    uint8_t iv[IV_SIZE];
    uint8_t iv_copy[IV_SIZE];
    uint8_t* padded = (uint8_t*)malloc(padded_len);
    uint8_t* encrypted = (uint8_t*)malloc(padded_len);

    memcpy(padded, text, text_len);
    add_pkcs7_padding(padded, text_len);

    // Шифрование с сохранением оригинального IV
    esp_fill_random(iv, IV_SIZE);
    memcpy(iv_copy, iv, IV_SIZE);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aes_key, KEY_SIZE * 8);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded, encrypted);

    // Запись в файл
    FILE* f = fopen("/spiffs/data.bin", "ab");
    if (f) {
        RecordHeader header = {.data_len = padded_len};
        memcpy(header.iv, iv, IV_SIZE);

        fwrite(&header, 1, sizeof(RecordHeader), f);
        fwrite(encrypted, 1, padded_len, f);
        fclose(f);
    }

    free(padded);
    free(encrypted);
    mbedtls_aes_free(&aes);
}

void getText(size_t record_num) {
    FILE* f = fopen("/spiffs/data.bin", "rb");
    if (!f) return;

    uint32_t current = 0;
    while (1) {
        RecordHeader header;
        // Чтение заголовка
        if (fread(&header, 1, sizeof(RecordHeader), f) != sizeof(RecordHeader))
            break;

        // Пропуск данных если не нужная запись
        if (current++ != record_num) {
            fseek(f, header.data_len, SEEK_CUR);
            continue;
        }

        // Чтение данных
        uint8_t* encrypted = static_cast<uint8_t*>(malloc(header.data_len));
        fread(encrypted, 1, header.data_len, f);

        // Дешифровка
        uint8_t* decrypted = static_cast<uint8_t*>(malloc(header.data_len));
        uint8_t iv_copy[IV_SIZE];
        memcpy(iv_copy, header.iv, IV_SIZE);

        mbedtls_aes_context aes;
        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_dec(&aes, aes_key, KEY_SIZE * 8);
        mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, header.data_len, iv_copy,
                              encrypted, decrypted);

        // Вывод
        // size_t plain_len = remove_pkcs7_padding(decrypted, header.data_len);
        log_i("%s", decrypted);

        free(decrypted);
        free(encrypted);
        mbedtls_aes_free(&aes);
        break;
    }
    fclose(f);
}

/*
int encrypt_data(const uint8_t* input, size_t ilen, uint8_t* output, uint8_t*
iv)
{
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    esp_fill_random(iv, IV_SIZE);

    int ret = mbedtls_aes_setkey_enc(&aes, aes_key, KEY_SIZE * 8);
    if (ret != 0) {
        ESP_LOGE(TAG, "AES key setup error: 0x%X", -ret);
        mbedtls_aes_free(&aes);
        return ret;
    }
    uint8_t iv_copy[IV_SIZE];
    memcpy(iv_copy, iv, IV_SIZE); // Копируем IV в выходной буфер
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, ilen, iv_copy, input,
output);

    mbedtls_aes_free(&aes);
    return ret;
}

int decrypt_data(const uint8_t* input, size_t ilen, uint8_t* output, const
uint8_t* iv)
{
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    int ret = mbedtls_aes_setkey_dec(&aes, aes_key, KEY_SIZE * 8);
    if (ret != 0) {
        ESP_LOGE(TAG, "AES key setup failed: %d", ret);
        return ret;
    }
    uint8_t iv_copy[IV_SIZE];
    memcpy(iv_copy, iv, IV_SIZE);
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ilen, iv_copy, input,
output); mbedtls_aes_free(&aes); return ret;
}
*/

size_t remove_pkcs7_padding(uint8_t* data, size_t len) {
    if (len == 0 || len % 16 != 0) return 0;
    uint8_t pad = data[len - 1];
    if (pad == 0 || pad > 16) return len;

    for (int i = len - pad; i < len; i++) {
        if (data[i] != pad) {
            // ESP_LOGW(TAG, "Invalid padding byte at %d: 0x%02X", i, data[i]);
            return len;
        }
    }
    return len - pad;
}

void add_pkcs7_padding(uint8_t* data, size_t len) {
    uint8_t pad = 16 - (len % 16);
    memset(data + len, pad, pad);
}

inline int_fast16_t calculate_padded_length(int_fast16_t len) {
    return ((len + 15) / 16) * 16;
}