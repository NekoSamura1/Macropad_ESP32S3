//?##################################################################################
//*         Config
// #define DEBUG

//?##################################################################################
//*         includes

#include <stdint.h>
#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <SPIFFS.h>
#include "ButtonPad/ButtonPad.h"
#include "Macros/Macros.h"

#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

//?##################################################################################
//*         preprocessor
#ifdef DEBUG
#define DEBUGSERIAL Serial0
#endif

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

enum BUTTONS
{
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

enum TABS
{
    TAB_A,
    TAB_B,
    TAB_C,
    TAB_D,
    TAB_COUNT,
};

enum MODES
{
    MODE_OFF,
    MODE_ON,
    MODE_SETTINGS,
};

#ifdef DEBUG
#define PRINT(x) DEBUGSERIAL.print(x)
#define PRINTLN(x) DEBUGSERIAL.println(x)
#define PRINT2(x, y) DEBUGSERIAL.print(x, y)
#define PRINTLN2(x, y) DEBUGSERIAL.println(x, y)

#define DUMPFS()                             \
    {                                        \
        PRINTLN("SPIFFS file list: ");       \
        fs::File root = SPIFFS.open("/");    \
        fs::File file = root.openNextFile(); \
        while (file)                         \
        {                                    \
            PRINT("\tFILE: ");               \
            PRINTLN(file.name());            \
            file = root.openNextFile();      \
        }                                    \
    }
#else
#define PRINT(x)
#define PRINTLN(x)
#define PRINT2(x, y)
#define PRINTLN2(x, y)
#define DUMPFS()
#endif

struct macros_t
{
    std::string imageName = "";
};

//?##################################################################################
//*         Globals

Macros *macrosOnTabs[TAB_COUNT][MACROS_COUNT_ON_TAB]{};
TABS currTab = TAB_A;
MODES currMode = MODE_ON;

USBHIDMouse Mouse;
USBHIDKeyboard Keyboard;

//?##################################################################################
//*         prototypes
void setBrightness(uint8_t brightness);
void handleScreen(void *args);
void handleButtons(void *args);
void mainSystem(void *args);

void debug(void *args);

void macrosInit();

int_fast8_t macroNumToButtonNum(int_fast8_t num);

//?##################################################################################
//*         macroses itself
void lmbSpam();
void rmbSpam();


//?##################################################################################
//*         macroses variants
Macros *macros_autoClickerLMB = new Macros(lmbSpam, MACROS_CYCLIC, 20 * MS_TO_US);
Macros *macros_hold_autoClickerLMB = new Macros(lmbSpam, MACROS_CYCLIC_TOGGLE, 20 * MS_TO_US);
Macros *macros_autoClickerRMB = new Macros(rmbSpam, MACROS_CYCLIC, 20 * MS_TO_US);
Macros *macros_hold_autoClickerRMB = new Macros(rmbSpam, MACROS_CYCLIC_TOGGLE, 20 * MS_TO_US);

#pragma once