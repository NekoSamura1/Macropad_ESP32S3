#pragma once

#define MAX_COMMAND_LENGTH 128
#define KEY_SIZE 32
#define SERIAL_CLI Serial0
#define MAX_INPUT_BUFFER 64

const char deviceName[] = "esp32";

#define FILE_RAW_PATH "/plain.bin"
#define FILE_ENC_PATH "/encrypt.bin"
#define TEMP_FILE_PATH "/temp_records.bin"

/* clang-format off */
#define KEY                                             \
{                                                       \
    0xC3, 0x20, 0x2D, 0xC8, 0x48, 0x4E, 0x04, 0x2B,     \
    0x8A, 0xA3, 0xEB, 0x8A, 0x6C, 0xFE, 0xF9, 0x30,     \
    0x98, 0xFD, 0xCC, 0xA3, 0xC6, 0xC3, 0xB7, 0x1E,     \
    0x1A, 0xAB, 0x34, 0xED, 0x82, 0x34, 0x9B, 0xB9      \
} /* clang-format on */
