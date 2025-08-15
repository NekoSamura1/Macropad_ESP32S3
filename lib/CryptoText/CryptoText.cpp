#include "CryptoText.h"
//?##################################################################################
//*         TEXT functions

// Append a new record to the file

void appendRecord(const uint8_t* iv, const char* data, uint16_t data_len) {
    File file = SPIFFS.open(FILE_PATH, FILE_APPEND);
    if (!file) {
        log_e("Failed to open file for appending");
        return;
    }

    RecordHeader header;
    memcpy(header.iv, iv, IV_SIZE);
    header.data_len = data_len;

    if (file.write((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
        log_e("Failed to write header");
        file.close();
        return;
    }

    if (file.write((uint8_t*)data, data_len) != data_len) {
        log_e("Failed to write data");
        file.close();
        return;
    }

    log_i("Record appended");
    file.close();
}

void readRecord(uint32_t n) {
    File file = SPIFFS.open(FILE_PATH, FILE_READ);
    if (!file) {
        log_e("Failed to open file for reading");
        return;
    }

    uint32_t pos = 0;
    uint32_t recordIndex = 0;
    RecordHeader header;

    while (file.available() && recordIndex < n) {
        if (file.read((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to read header or EOF");
            file.close();
            return;
        }
        file.seek(pos + sizeof(RecordHeader) + header.data_len);
        pos += sizeof(RecordHeader) + header.data_len;
        recordIndex++;
    }

    if (recordIndex != n || !file.available()) {
        log_w("Record not found");
        file.close();
        return;
    }

    if (file.read((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
        log_e("Failed to read header");
        file.close();
        return;
    }

    char* data = (char*)malloc(header.data_len + 1);
    if (!data) {
        log_e("Memory allocation failed");
        file.close();
        return;
    }

    if (file.read((uint8_t*)data, header.data_len) != header.data_len) {
        log_e("Failed to read data");
        free(data);
        file.close();
        return;
    }
    data[header.data_len] = '\0';

    char iv_str[97]; // 32 bytes * 3 (2 hex + space) + 1 for null
    iv_str[0] = '\0';
    for (int i = 0; i < IV_SIZE; i++) {
        char temp[4];
        snprintf(temp, sizeof(temp), "%02X ", header.iv[i]);
        strlcat(iv_str, temp, sizeof(iv_str));
    }
    log_i("Record %u: iv=%sdata_len=%u data=%s", n, iv_str, header.data_len, data);

    free(data);
    file.close();
}

void deleteFile(const char* fileName) {
    if (SPIFFS.remove(fileName)) {
        log_i("File %s deleted", fileName);
    } else {
        log_e("Failed %s to delete file", fileName);
    }
}

void deleteRecord(uint32_t n) {
    File src = SPIFFS.open(FILE_PATH, FILE_READ);
    if (!src) {
        log_e("Failed to open source file");
        return;
    }

    File dst = SPIFFS.open(TEMP_FILE_PATH, FILE_WRITE);
    if (!dst) {
        log_e("Failed to open temp file");
        src.close();
        return;
    }

    uint32_t pos = 0;
    uint32_t recordIndex = 0;
    RecordHeader header;
    uint8_t buffer[128];

    while (src.available() && recordIndex < n) {
        if (src.read((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to read header or EOF");
            src.close();
            dst.close();
            SPIFFS.remove(TEMP_FILE_PATH);
            return;
        }
        if (dst.write((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to write header to temp");
            src.close();
            dst.close();
            SPIFFS.remove(TEMP_FILE_PATH);
            return;
        }

        uint16_t remaining = header.data_len;
        while (remaining > 0) {
            size_t toRead = min(remaining, (uint16_t)sizeof(buffer));
            if (src.read(buffer, toRead) != toRead) {
                log_e("Failed to read data");
                src.close();
                dst.close();
                SPIFFS.remove(TEMP_FILE_PATH);
                return;
            }
            if (dst.write(buffer, toRead) != toRead) {
                log_e("Failed to write data to temp");
                src.close();
                dst.close();
                SPIFFS.remove(TEMP_FILE_PATH);
                return;
            }
            remaining -= toRead;
        }
        pos += sizeof(RecordHeader) + header.data_len;
        recordIndex++;
    }

    if (recordIndex != n || !src.available()) {
        log_w("Record not found");
        src.close();
        dst.close();
        SPIFFS.remove(TEMP_FILE_PATH);
        return;
    }

    if (src.read((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
        log_e("Failed to read header to skip");
        src.close();
        dst.close();
        SPIFFS.remove(TEMP_FILE_PATH);
        return;
    }
    src.seek(pos + sizeof(RecordHeader) + header.data_len);

    while (src.available()) {
        if (src.read((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to read header or EOF");
            break;
        }
        if (dst.write((uint8_t*)&header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to write header to temp");
            src.close();
            dst.close();
            SPIFFS.remove(TEMP_FILE_PATH);
            return;
        }

        uint16_t remaining = header.data_len;
        while (remaining > 0) {
            size_t toRead = min(remaining, (uint16_t)sizeof(buffer));
            if (src.read(buffer, toRead) != toRead) {
                log_e("Failed to read data");
                src.close();
                dst.close();
                SPIFFS.remove(TEMP_FILE_PATH);
                return;
            }
            if (dst.write(buffer, toRead) != toRead) {
                log_e("Failed to write data to temp");
                src.close();
                dst.close();
                SPIFFS.remove(TEMP_FILE_PATH);
                return;
            }
            remaining -= toRead;
        }
    }

    src.close();
    dst.close();

    if (!SPIFFS.remove(FILE_PATH)) {
        log_e("Failed to delete original file");
        SPIFFS.remove(TEMP_FILE_PATH);
        return;
    }
    if (!SPIFFS.rename(TEMP_FILE_PATH, FILE_PATH)) {
        log_e("Failed to rename temp file");
        return;
    }

    log_i("Record deleted");
}
// void addRecord(const char* text) {
//     size_t text_len = strlen(text);
//     uint16_t padded_len = calculate_padded_length(text_len);

//     // Подготовка данных
//     uint8_t iv[IV_SIZE];
//     uint8_t iv_copy[IV_SIZE];
//     uint8_t* padded = (uint8_t*)malloc(padded_len);
//     uint8_t* encrypted = (uint8_t*)malloc(padded_len);

//     memcpy(padded, text, text_len);
//     add_pkcs7_padding(padded, text_len);

//     // Шифрование с сохранением оригинального IV
//     esp_fill_random(iv, IV_SIZE);
//     memcpy(iv_copy, iv, IV_SIZE);

//     mbedtls_aes_context aes;
//     mbedtls_aes_init(&aes);
//     mbedtls_aes_setkey_enc(&aes, aes_key, KEY_SIZE * 8);
//     mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded, encrypted);

//     // Запись в файл
//     FILE* f = fopen("/spiffs/data.bin", "ab");
//     if (f) {
//         RecordHeader header = {.data_len = padded_len};
//         memcpy(header.iv, iv, IV_SIZE);

//         fwrite(&header, 1, sizeof(RecordHeader), f);
//         fwrite(encrypted, 1, padded_len, f);
//         fclose(f);
//     }
//     log_i("added record: %s", text);
//     free(padded);
//     free(encrypted);
//     mbedtls_aes_free(&aes);
// }

// void getRecord(size_t record_num) {
//     log_i("Reading record :%lu", record_num);

//     FILE* f = fopen("/spiffs/data.bin", "rb");
//     if (!f) return;

//     uint32_t current = 0;
//     while (1) {
//         RecordHeader header;
//         // Чтение заголовка
//         if (fread(&header, 1, sizeof(RecordHeader), f) != sizeof(RecordHeader))
//             break;

//         // Пропуск данных если не нужная запись
//         if (current++ != record_num) {
//             fseek(f, header.data_len, SEEK_CUR);
//             continue;
//         }

//         // Чтение данных
//         uint8_t* encrypted = static_cast<uint8_t*>(malloc(header.data_len));
//         fread(encrypted, 1, header.data_len, f);

//         // Дешифровка
//         uint8_t* decrypted = static_cast<uint8_t*>(malloc(header.data_len));
//         uint8_t iv_copy[IV_SIZE];
//         memcpy(iv_copy, header.iv, IV_SIZE);

//         mbedtls_aes_context aes;
//         mbedtls_aes_init(&aes);
//         mbedtls_aes_setkey_dec(&aes, aes_key, KEY_SIZE * 8);
//         mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, header.data_len, iv_copy,
//                               encrypted, decrypted);

//         // Вывод
//         // size_t plain_len = remove_pkcs7_padding(decrypted, header.data_len);
//         log_i("%s", decrypted);

//         free(decrypted);
//         free(encrypted);
//         mbedtls_aes_free(&aes);
//         break;
//     }
//     fclose(f);
// }

// /*
// int encrypt_data(const uint8_t* input, size_t ilen, uint8_t* output, uint8_t*
// iv)
// {
//     mbedtls_aes_context aes;
//     mbedtls_aes_init(&aes);

//     esp_fill_random(iv, IV_SIZE);

//     int ret = mbedtls_aes_setkey_enc(&aes, aes_key, KEY_SIZE * 8);
//     if (ret != 0) {
//         ESP_LOGE(TAG, "AES key setup error: 0x%X", -ret);
//         mbedtls_aes_free(&aes);
//         return ret;
//     }
//     uint8_t iv_copy[IV_SIZE];
//     memcpy(iv_copy, iv, IV_SIZE); // Копируем IV в выходной буфер
//     ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, ilen, iv_copy, input,
// output);

//     mbedtls_aes_free(&aes);
//     return ret;
// }

// int decrypt_data(const uint8_t* input, size_t ilen, uint8_t* output, const
// uint8_t* iv)
// {
//     mbedtls_aes_context aes;
//     mbedtls_aes_init(&aes);
//     int ret = mbedtls_aes_setkey_dec(&aes, aes_key, KEY_SIZE * 8);
//     if (ret != 0) {
//         ESP_LOGE(TAG, "AES key setup failed: %d", ret);
//         return ret;
//     }
//     uint8_t iv_copy[IV_SIZE];
//     memcpy(iv_copy, iv, IV_SIZE);
//     ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ilen, iv_copy, input,
// output); mbedtls_aes_free(&aes); return ret;
// }
// */

// size_t remove_pkcs7_padding(uint8_t* data, size_t len) {
//     if (len == 0 || len % 16 != 0) return 0;
//     uint8_t pad = data[len - 1];
//     if (pad == 0 || pad > 16) return len;

//     for (int i = len - pad; i < len; i++) {
//         if (data[i] != pad) {
//             // ESP_LOGW(TAG, "Invalid padding byte at %d: 0x%02X", i, data[i]);
//             return len;
//         }
//     }
//     return len - pad;
// }

// void add_pkcs7_padding(uint8_t* data, size_t len) {
//     uint8_t pad = 16 - (len % 16);
//     memset(data + len, pad, pad);
// }

// inline uint_fast16_t calculate_padded_length(int_fast16_t len) {
//     return ((len + 15) / 16) * 16;
// }