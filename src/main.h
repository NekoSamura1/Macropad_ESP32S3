//?##################################################################################
//*         Config

//?##################################################################################
//*         includes
#include "ButtonPad.h"
#include "CryptoText.h"
#include "Macros.h"
#include "cli.h"
#include "config.h"

#include <Arduino.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <USBHIDMouse.h>
#include <stdint.h>

//?##################################################################################
//*         preprocessor

#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE != 0
#error Should be copiled when USB is in OTG mode
#endif

//?##################################################################################
//*         defines

#define MS_TO_US 1000
#define PIN_BACKLIGHT 9
#define MACROS_COUNT_ON_TAB 10

#define BUTTON_MASK(num) (1 << (num))

enum BUTTONS {
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_A,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_B,
    BUTTON_7,
    BUTTON_8,
    BUTTON_9,
    BUTTON_C,
    BUTTON_STAR,
    BUTTON_0,
    BUTTON_BARS,
    BUTTON_D,
    BUTTON_COUNT,
};

enum TABS {
    TAB_A,
    TAB_B,
    TAB_C,
    TAB_D,
    TAB_COUNT,
};

enum MODES {
    MODE_OFF,
    MODE_ON,
    MODE_SETTINGS,
};

enum COMMANDS {
    C_NaC,
    C_ADD_STRING,
    C_READ_STRING,
};

#define DUMPFS()                                                                                                                                                                                       \
    {                                                                                                                                                                                                  \
        log_i("SPIFFS file list:");                                                                                                                                                                    \
        fs::File root = SPIFFS.open("/");                                                                                                                                                              \
        fs::File file = root.openNextFile();                                                                                                                                                           \
        while (file) {                                                                                                                                                                                 \
            log_i("FILE: %s", file.name());                                                                                                                                                            \
            file = root.openNextFile();                                                                                                                                                                \
        }                                                                                                                                                                                              \
    }

// struct macros_t {
//     std::string imageName = "";
// };

//?##################################################################################
//*         Globals

Macros* macrosOnTabs[TAB_COUNT][MACROS_COUNT_ON_TAB]{};
TABS currTab = TAB_A;
MODES currMode = MODE_ON;

USBHIDMouse Mouse;
USBHIDKeyboard Keyboard;

//?##################################################################################
//*         prototypes
void setBrightness(uint8_t brightness);
void handleScreen(void* args);
void handleButtons(void* args);
void mainSystem(void* args);
void macrosInit();

int_fast8_t macrosNumToButtonNum(int_fast8_t num);
int_fast8_t buttonNumToMacrosNum(int_fast8_t num);

//?##################################################################################
//*         macroses itself
void lmbSpam(); // just click LMB
void rmbSpam(); // just click RMB

void plusW();      // press w
void minusW();     // unpress w
void powershell(); // open powershell

void printEncrypted(size_t n);
void printSaved(size_t n);

//?##################################################################################
//*         macroses variants
Macros* macros_autoClickerLMB = new Macros(lmbSpam, nullptr, MACROS_CYCLIC, 20 * MS_TO_US);
Macros* macros_toggle_autoClickerLMB = new Macros(lmbSpam, nullptr, MACROS_CYCLIC_TOGGLE, 20 * MS_TO_US);
Macros* macros_autoClickerRMB = new Macros(rmbSpam, nullptr, MACROS_CYCLIC, 20 * MS_TO_US);
Macros* macros_toggle_autoClickerRMB = new Macros(rmbSpam, nullptr, MACROS_CYCLIC_TOGGLE, 20 * MS_TO_US);
// Macros *macros_plusW = new Macros(plusW, minusW, MACROS_HOLD, 0);
Macros* macros_toggle_plusW = new Macros(plusW, minusW, MACROS_HOLD_TOGGLE, 0);
Macros* macros_powershell_call = new Macros(powershell, nullptr, MACROS_ONCE, 0);

// Macros* macros_print = new Macros(powershell, nullptr, MACROS_ONCE, 0);
// Macros* macros_powershell_call = new Macros(powershell, nullptr, MACROS_ONCE, 0);

//?##################################################################################
//*         cli commands
char inputBuffer[MAX_INPUT_BUFFER];

void cmdEcho(const char* arg);
void cmdLed(const char* arg);
void cmdLS(const char* arg);

void cmdReadRecord(const char* arg);
void cmdAddRecord(const char* arg);
void cmdDeleteRecord(const char* arg);
void cmdDeleteFile(const char* arg);

void cmdAddEncrypt(const char* arg);
void cmdGetEncrypt(const char* arg);

Command commands[] = {
    {"echo", cmdEcho},               //
    {"led", cmdLed},                 //
    {"ls", cmdLS},                   //
    {"addText", cmdAddRecord},       //
    {"getText", cmdReadRecord},      //
    {"deleteText", cmdDeleteRecord}, //
    {"deleteFile", cmdDeleteFile},   //
    // {"test", testEncryptDecrypt},    //
    {"addEncrypted", cmdAddEncrypt}, //
    {"getEncrypted", cmdGetEncrypt}, //
};
