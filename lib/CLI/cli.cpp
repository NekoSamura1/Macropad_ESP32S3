
#include "cli.h"

const Command* commandList;
size_t commandCount{};
HardwareSerial* Serial_Cli;

void initCLI(HardwareSerial* _Serial_Cli, const Command* const command, const size_t size) {
    delay(10);
    commandList = command;
    commandCount = size / sizeof(Command);
    Serial_Cli = _Serial_Cli;
    Serial_Cli->begin(115200);
}

void processCommand(char* const inputBuffer, const char* const prefix) {

    static char cmdBuffer[MAX_COMMAND];
    static char argBuffer[MAX_ARG];

    if (strncmp(inputBuffer, prefix, strlen(prefix)) != 0) {
        return;
    }
    Serial_Cli->println(inputBuffer);

    char* input = &inputBuffer[strlen(prefix)];
    while (*input == ' ')
        input++;

    char* space = strchr(input, ' ');
    cmdBuffer[0] = '\0';
    argBuffer[0] = '\0';

    if (space) {
        int cmdLen = space - input;
        if (cmdLen < MAX_COMMAND) {
            strncpy(cmdBuffer, input, cmdLen);
            cmdBuffer[cmdLen] = '\0';
        }
        while (*space == ' ')
            space++;
        if (strlen(space) < MAX_ARG) {
            strcpy(argBuffer, space);
        }
    } else if (strlen(input) < MAX_COMMAND) {
        strcpy(cmdBuffer, input);
    }

    for (size_t i = 0; i < commandCount; i++) {
        if (strcasecmp(cmdBuffer, commandList[i].name) == 0) {
            commandList[i].function(argBuffer);
            break;
        } else if (i == commandCount - 1) {
            log_i("No such command available");
        }
    }
}

void processCLI(char* inputBuffer, const size_t bufferSize, const char* const prefix) {
    static size_t bufferIndex{};
    while (Serial_Cli->available() && bufferIndex < bufferSize - 1) {
        char c = Serial_Cli->read();

        if (c == '\b' || c == 0x7F) { // Handle backspace
            if (bufferIndex > 0) {
                // Check if we're deleting a multi-byte UTF-8 character
                int bytesToDelete = 1;
                if (bufferIndex >= 2 && (inputBuffer[bufferIndex - 1] & 0xC0) == 0x80) {
                    // Continuation byte, check for 2, 3, or 4-byte sequence
                    if (bufferIndex >= 2 && (inputBuffer[bufferIndex - 2] & 0xE0) == 0xC0) {
                        bytesToDelete = 2;
                    } else if (bufferIndex >= 3 && (inputBuffer[bufferIndex - 3] & 0xF0) == 0xE0) {
                        bytesToDelete = 3;
                    } else if (bufferIndex >= 4 && (inputBuffer[bufferIndex - 4] & 0xF8) == 0xF0) {
                        bytesToDelete = 4;
                    }
                }
                bufferIndex -= bytesToDelete;
                if (bufferIndex < 0)
                    bufferIndex = 0;
                // for (int i = 0; i < bytesToDelete; i++) {
                Serial_Cli->print("\b \b"); // Erase character(s) on terminal
                // }
            }
            continue;
        }

        Serial_Cli->write(c); // Echo character back
        if (c == '\n' || c == '\r') {
            inputBuffer[bufferIndex] = '\0';
            processCommand(inputBuffer, prefix);
            bufferIndex = 0;
        } else {
            inputBuffer[bufferIndex++] = c; // Store all bytes (ASCII and UTF-8)
        }
    }
}