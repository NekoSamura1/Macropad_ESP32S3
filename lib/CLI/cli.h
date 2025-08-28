#pragma once

#include <Arduino.h>
#include <stdint.h>

#define MAX_DEVICE_NAME 16
#define MAX_COMMAND 16
#define MAX_ARG 32
#define KEY_SIZE 32

struct Command {
    const char* name;
    void (*function)(const char* arg);
};

void initCLI(HardwareSerial* _Serial_Cli, const Command* const command, const size_t size);
void processCLI(char* inputBuffer, const size_t bufferSize, const char* const prefix);
