#include "main.h"
// clang-format off or /* clang-format off */
// clang-format on or /* clang-format on */

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
uint16_t buttonState = 0;

uint8_t brightness = 100;

void setup(void) {
    log_i("Start");

    //* start SPIFFS
    while (!SPIFFS.begin()) {
        log_e("SPIFFS not started, retrying...");
        vTaskDelay(100);
    }
    DUMPFS();

    //*init CLI
    log_d("CLI setup");
    initCLI(&SERIAL_CLI, commands, sizeof(commands));

    //* setup button pad
    log_d("Button pad setup");
    buttonStateInit();

    //* init usb as hid
    log_d("USB setup");
    Mouse.begin();
    Keyboard.begin();
    if (USB.begin()) {
        log_e("USB not started");
    }

    //* initialize macroses
    log_d("Macroses setup");
    macrosInit();

    //* setup backlight pwm
    log_d("Backlight setup");
    pinMode(PIN_BACKLIGHT, OUTPUT);
    ledcSetup(0, 100000, 8);
    ledcAttachPin(PIN_BACKLIGHT, 0);

    //* setup display
    log_d("Display setup");
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(0);
    tft.loadFont("DaysOne16");
    tft.setCursor(8, 5);
    tft.print("ButtonState");
    setBrightness(brightness);

    //* task create
    log_d("Task create");
    xTaskCreate(handleScreen, "handleScreen", 4096, NULL, 1, NULL);

    xTaskCreate(handleButtons, "handleButtons", 1024, NULL, 1, NULL);

    xTaskCreate(mainSystem, "mainSystem", 32768, NULL, 1, NULL);
}

void loop() {
    log_i("loop deleted");
    vTaskDelete(NULL);
}

void handleScreen(void* args) {
    for (;;) {
        setBrightness(currMode == MODE_OFF ? 0 : brightness); // turn on/off the backlight

        switch (currMode) {
        case MODE_OFF:
            setBrightness(0);
            break;
        case MODE_ON:
            //* Button state
            for (size_t buttonRow = 0; buttonRow < 4; buttonRow++) {
                for (size_t buttonColumn = 0; buttonColumn < 4; buttonColumn++) {
                    // button state grid topright corner
                    tft.drawPixel(121 + buttonColumn * 2, 0 + buttonRow * 2, (buttonState & (1 << (buttonColumn + buttonRow * 4))) ? TFT_GREEN : TFT_RED); // display buttons state
                }
            }

            for (size_t macroRow = 0; macroRow < 3; macroRow++) {
                for (size_t macroColumn = 0; macroColumn < 3; macroColumn++) {
                    bool currMacsosState = false;
                    int_fast8_t macrosID;
                    macrosID = macroRow * 4 + macroColumn;

                    if (macrosOnTabs[currTab][macrosID] != nullptr) {
                        currMacsosState = macrosOnTabs[currTab][macrosID]->getStatus();
                    }

                    // draw itself
                    tft.drawCircle(34 + 28 * macroColumn, 50 + 28 * macroRow, 11, currMacsosState ? TFT_GREEN : TFT_RED); // display macroses state
                    tft.drawCircle(34 + 28 * macroColumn, 50 + 28 * macroRow, 10, currMacsosState ? TFT_GREEN : TFT_RED);
                }
            }
            tft.drawCircle(34 + 28, 50 + 28 * 3, 11, ((macrosOnTabs[currTab][9] != nullptr) ? macrosOnTabs[currTab][9]->getStatus() : false) ? TFT_GREEN : TFT_RED);
            tft.drawCircle(34 + 28, 50 + 28 * 3, 10, ((macrosOnTabs[currTab][9] != nullptr) ? macrosOnTabs[currTab][9]->getStatus() : false) ? TFT_GREEN : TFT_RED);

            //* tab slider
            if (currTab != 0) {
                tft.fillRect(0, 154, ((currTab) << 5) - 1, 5, TFT_DARKGREY);
            }
            tft.fillRect((currTab) << 5, 154, 32, 5, TFT_RED);
            if (currTab != 3) {
                tft.fillRect((currTab + 1) << 5, 154, (3 - currTab) << 5, 5, TFT_DARKGREY);
            }

            break;
        case MODE_SETTINGS:
            setBrightness(0);
            break;

        default:
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void handleButtons(void* args) {
    for (;;) {
        buttonStateUpdate(&buttonState);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void mainSystem(void* args) {
    for (;;) {
        switch (currMode) {
        case MODE_ON:
            if (buttonState == 1 << BUTTON_A) {
                currTab = TAB_A;
            } else if (buttonState == 1 << BUTTON_B) {
                currTab = TAB_B;
            } else if (buttonState == 1 << BUTTON_C) {
                currTab = TAB_C;
            } else if (buttonState == 1 << BUTTON_D) {
                currTab = TAB_D;
            } else if (buttonState & 1 << BUTTON_STAR) {
                log_i("MODE_OFF");
                currMode = MODE_OFF;
                while ((buttonState & 1 << BUTTON_STAR)) {
                    vTaskDelay(1 / portTICK_PERIOD_MS);
                    buttonStateUpdate(&buttonState);
                }
            }

            switch (currTab) // macros activation
            {
            case TAB_A:
            case TAB_B:
            case TAB_C:
            case TAB_D: {
                for (size_t i = 0; i < MACROS_COUNT_ON_TAB; i++) {
                    if (macrosOnTabs[currTab][i] != nullptr) {
                        macrosOnTabs[currTab][i]->pokeMacro(buttonState & 1 << macrosNumToButtonNum(i));
                        macrosOnTabs[currTab][i]->runMacro();
                    }
                }
                break;
            }
            default:
                break;
            }
            break;
        case MODE_SETTINGS:
            /* code */
            break;
        case MODE_OFF:
        default:
            if ((buttonState & 1 << BUTTON_STAR)) {
                log_i("MODE_ON");
                currMode = MODE_ON;
            }
            while ((buttonState & 1 << BUTTON_STAR)) {
                vTaskDelay(1 / portTICK_PERIOD_MS);
                buttonStateUpdate(&buttonState);
            }

            if (buttonState & BUTTON_MASK(BUTTON_BARS)) {
                uint16_t savedButtonState = buttonState & ~BUTTON_MASK(BUTTON_BARS);
                size_t n = log2(savedButtonState);
                if (n < BUTTON_COUNT) {
                    log_i("printEncrypted(%d)", n);
                    printEncrypted(n);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
            } else if (buttonState & BUTTON_MASK(BUTTON_D)) {
                uint16_t savedButtonState = buttonState & ~BUTTON_MASK(BUTTON_D);
                size_t n = log2(savedButtonState);
                if (n < BUTTON_COUNT) {
                    log_i("printSaved(%d)", n);
                    printSaved(n);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
            }
            processCLI(inputBuffer, sizeof(inputBuffer), deviceName); //! fixme should be in "mode off"
            break;
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void macrosInit() {
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_1)] = macros_autoClickerLMB;
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_4)] = macros_toggle_autoClickerLMB;
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_2)] = macros_autoClickerRMB;
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_5)] = macros_toggle_autoClickerRMB;
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_3)] = macros_powershell_call;
    macrosOnTabs[TAB_A][buttonNumToMacrosNum(BUTTON_6)] = macros_toggle_plusW;
}

void setBrightness(uint8_t brightness) { ledcWrite(0, brightness); }

//?##################################################################################
//*         helper functions

int_fast8_t macrosNumToButtonNum(int_fast8_t num) {
    assert(0 <= num and num < MACROS_COUNT_ON_TAB);
    switch (num) {
    case 9:
        num = BUTTON_0;
        break;
    default:
        num = num * 4 / 3;
        break;
    }
    return num;
}

int_fast8_t buttonNumToMacrosNum(int_fast8_t num) {
    assert(0 <= num and num < 16);
    switch (num) {
    case BUTTON_0:
        num = 9;
        break;
    case BUTTON_1:
    case BUTTON_2:
    case BUTTON_3:
    case BUTTON_4:
    case BUTTON_5:
    case BUTTON_6:
    case BUTTON_7:
    case BUTTON_8:
    case BUTTON_9:
        num = (num + 2) * 3 / 4 - 1;
        break;
    default:
        num = -1;
        break;
    }
    return num;
}


// void handleSequenceInput(char key) {
//   if (sequenceIndex < sizeof(inputSequence) - 1) {
//     inputSequence[sequenceIndex++] = key;
//     inputSequence[sequenceIndex] = '\0';
//     digitalWrite(LED_BUILTIN, HIGH);
//     delay(100);
//     digitalWrite(LED_BUILTIN, LOW);
//     log_i("Key %c entered", key);
//   }
//   if (key == '0' || key == '#') { // End sequence on '0' or '#'
//     if (strcmp(inputSequence, correctSequence) == 0) {
//       encryptedUnlocked = true;
//       log_i("Encrypted records unlocked");
//     } else {
//       log_e("Incorrect sequence");
//     }
//     sequenceIndex = 0;
//     memset(inputSequence, 0, sizeof(inputSequence));
//   }
// }

// void lookupForPrintable(uint16_t localButtonState) {
//     size_t n = log2(savedButtonState);
//     if (n < BUTTON_COUNT) {
//         log_i("printEncrypted(%d)", n);
//         printEncrypted(n);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

//?##################################################################################
//*         macroses itself

void lmbSpam() { Mouse.click(MOUSE_LEFT); }

void rmbSpam() { Mouse.click(MOUSE_RIGHT); }

void plusW() { Keyboard.press('w'); }

void minusW() { Keyboard.release('w'); }

void powershell() {
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');
    Keyboard.release(KEY_LEFT_GUI);
    Keyboard.release('r');
    delay(100);
    Keyboard.print("powershell");
    Keyboard.press(KEY_RETURN);
    Keyboard.release(KEY_RETURN);
}

void printEncrypted(size_t n) {
    char* record = nullptr;
    readEncryptedRecord(n, &record);
    if (record != nullptr) {
        log_i("Record print: %s", record);
        Keyboard.print(record);
        free(record);
    }
}

void printSaved(size_t n) {
    RecordHeader header;
    char* record = nullptr;
    readRecordRaw(n, &record, &header);
    if (record != nullptr) {
        log_i("Record print: %s", record);
        Keyboard.print(record);
        free(record);
    }
}

//?##################################################################################
//*         cli commands

void cmdLed(const char* arg) {
    if (strcasecmp(arg, "on") == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        SERIAL_CLI.println("LED ON");
    } else if (strcasecmp(arg, "off") == 0) {
        digitalWrite(LED_BUILTIN, LOW);
        SERIAL_CLI.println("LED OFF");
    } else {
        SERIAL_CLI.println("Invalid LED argument");
    }
}

void cmdEcho(const char* arg) { SERIAL_CLI.println(arg); }
void cmdLS(const char* arg) { DUMPFS(); }

void cmdReadRecord(const char* arg) {
    size_t recordNumber;
    recordNumber = atoi(arg);
    RecordHeader header;
    uint8_t* record;
    readRecordRaw(recordNumber, &record, &header);
    log_i("len = %d, iv = %i, data = %s", header.data_len, header.iv[0], record);
    free(record);
}

void cmdAddRecord(const char* arg) {
    const uint8_t tempIV[IV_SIZE]{};
    appendRecordRaw(tempIV, reinterpret_cast<const uint8_t*>(arg), strlen(arg) + 1);
}

void cmdDeleteRecord(const char* arg) {
    size_t recordNumber;
    recordNumber = atoi(arg);
    deleteRecord(recordNumber);
}

void cmdDeleteFile(const char* arg) { deleteFile(arg); }

void cmdAddEncrypted(const char* arg) { appendEncryptedRecord(arg); }

void cmdReadEncrypted(const char* arg) {
    char* text = nullptr;
    readEncryptedRecord(atoi(arg), &text);
    if (text)
        free(text);
}