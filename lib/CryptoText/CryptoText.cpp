#include "CryptoText.h"
//?##################################################################################
//*         TEXT functions

void appendRecordRaw(const uint8_t* iv, const uint8_t* data, uint16_t data_len) {
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

void readRecordRaw(uint32_t n, uint8_t** data, RecordHeader* header) {
    File file = SPIFFS.open(FILE_PATH, FILE_READ);
    if (!file) {
        log_e("Failed to open file for reading");
        return;
    }

    uint32_t pos = 0;
    uint32_t recordIndex = 0;

    while (file.available() && recordIndex < n) {
        if (file.read((uint8_t*)header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
            log_e("Failed to read header or EOF");
            file.close();
            return;
        }
        file.seek(pos + sizeof(RecordHeader) + header->data_len);
        pos += sizeof(RecordHeader) + header->data_len;
        recordIndex++;
    }

    if (recordIndex != n || !file.available()) {
        log_w("Record not found");
        file.close();
        return;
    }

    if (file.read((uint8_t*)header, sizeof(RecordHeader)) != sizeof(RecordHeader)) {
        log_e("Failed to read header");
        file.close();
        return;
    }

    (*data) = (uint8_t*)malloc(header->data_len + 1);
    if (!(*data)) {
        log_e("Memory allocation failed");
        file.close();
        return;
    }

    if (file.read((uint8_t*)(*data), header->data_len) != header->data_len) {
        log_e("Failed to read (*data)");
        free((*data));
        file.close();
        return;
    }
    (*data)[header->data_len] = '\0';

    char iv_str[49]; // 16 bytes * 3 (2 hex + space) + 1 for null
    iv_str[0] = '\0';
    for (int i = 0; i < IV_SIZE; i++) {
        char temp[4];
        snprintf(temp, sizeof(temp), "%02X ", header->iv[i]);
        strlcat(iv_str, temp, sizeof(iv_str));
    }
    log_i("Record %u: iv = %s, data_len = %u, (*data) = %s", n, iv_str, header->data_len, (*data));
    file.close();
    log_i("readed");
    delay(100);
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

int encrypt_data(size_t inputLen, uint8_t* iv, const uint8_t* input, uint8_t* output) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    esp_fill_random(iv, IV_SIZE);

    int ret = mbedtls_aes_setkey_enc(&aes, aes_key, KEY_SIZE * 8);
    if (ret != 0) {
        log_e("AES key setup error: 0x%X", -ret);
        mbedtls_aes_free(&aes);
        return ret;
    }

    uint8_t iv_copy[IV_SIZE];
    memcpy(iv_copy, iv, IV_SIZE);
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, inputLen, iv_copy, input, output);

    mbedtls_aes_free(&aes);
    return ret;
}

int decrypt_data(size_t ilen, const uint8_t* iv, const uint8_t* input, uint8_t* output) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    int ret = mbedtls_aes_setkey_dec(&aes, aes_key, KEY_SIZE * 8);
    if (ret != 0) {
        log_e("AES key setup failed: %d", ret);
        return ret;
    }
    uint8_t iv_copy[IV_SIZE];
    memcpy(iv_copy, iv, IV_SIZE);
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ilen, iv_copy, input, output);
    mbedtls_aes_free(&aes);
    return ret;
}

size_t remove_pkcs7_padding(uint8_t* data, size_t len) {
    if (len == 0 || len % 16 != 0) {
        return 0;
    }
    uint8_t pad = data[len - 1];
    if (pad == 0 || pad > 16) {
        return len;
    }
    for (int i = len - pad; i < len; i++) {
        if (data[i] != pad) {
            log_w("Invalid padding byte at %d: 0x%02X", i, data[i]);
            return len;
        }
    }
    return len - pad;
}

void add_pkcs7_padding(uint8_t* data, size_t len) {
    uint8_t pad = 16 - (len % 16);
    memset(data + len, pad, pad);
}

inline size_t calculate_padded_length(size_t len) { return ((len + 15) / 16) * 16; }

void testEncryptDecrypt(const char* text) {

    size_t text_len = strlen(text) + 1;
    size_t padded_len = calculate_padded_length(text_len);

    // Подготовка данных
    uint8_t iv[IV_SIZE];

    uint8_t* padded = static_cast<uint8_t*>(malloc(padded_len));
    uint8_t* encrypted = static_cast<uint8_t*>(malloc(padded_len));

    memcpy(padded, text, text_len);
    add_pkcs7_padding(padded, text_len);

    encrypt_data(padded_len, iv, padded, encrypted);

    char iv_str[50]; // 32 bytes * 3 (2 hex + space) + 1 for null
    iv_str[0] = '\0';
    for (int i = 0; i < IV_SIZE; i++) {
        char temp[4];
        snprintf(temp, sizeof(temp), "%02X ", iv[i]);
        strlcat(iv_str, temp, sizeof(iv_str));
    }
    log_i("Record: iv = %s, data_len = %u, data = %s", iv_str, padded_len, padded);
    log_i("Encrypted = %s", encrypted);
    memset(padded, 0, padded_len);
    decrypt_data(padded_len, iv, encrypted, padded);

    log_i("Record: iv = %s data_len = %u data = %s", iv_str, padded_len, padded);
    remove_pkcs7_padding(padded, padded_len);
    log_i("Record: iv = %s data_len = %u data = %s", iv_str, text_len, padded);

    free(padded);
    free(encrypted);
}

void appendEncryptedRecord(const char* text) {

    size_t text_len = strlen(text) + 1;
    size_t padded_len = calculate_padded_length(text_len);

    // Подготовка данных
    uint8_t iv[IV_SIZE];

    uint8_t* padded = static_cast<uint8_t*>(malloc(padded_len));
    uint8_t* encrypted = static_cast<uint8_t*>(malloc(padded_len));

    memcpy(padded, text, text_len);
    add_pkcs7_padding(padded, text_len);

    encrypt_data(padded_len, iv, padded, encrypted);
    appendRecordRaw(iv, encrypted, padded_len);

    free(padded);
    free(encrypted);
}

void readEncryptedRecord(size_t number, char* text) {
    RecordHeader header{};
    uint8_t* encrypted;

    readRecordRaw(number, &encrypted, &header);
    delay(100);
    log_i("Record %u: data_len = %u data = %s", number, header.data_len, encrypted);
    delay(100);

    uint8_t* padded = static_cast<uint8_t*>(malloc(header.data_len));
    delay(100);
    decrypt_data(header.data_len, header.iv, encrypted, padded);
    delay(100);
    size_t unpadded_len = remove_pkcs7_padding(padded, header.data_len);
    padded[unpadded_len] = '\0';
    delay(100);
    log_i("Record: data_len = %u data = %s", header.data_len, padded);
    text = static_cast<char*>(malloc(unpadded_len + 1));
    snprintf(text, unpadded_len + 1, "%s", padded);
    log_i("Record: str_len = %u, unpadded_len = %u, data = %s", strlen(text), unpadded_len, text);
    free(encrypted);
    free(padded);
}
