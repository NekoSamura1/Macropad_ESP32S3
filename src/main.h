//?##################################################################################
//*         Config
#define DEBUG
#define DEBUGSERIAL Serial0

//?##################################################################################
//*         includes

#include <stdint.h>
#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <SPIFFS.h>

#include "ButtonPad/ButtonPad.h"

//?##################################################################################
//*         defines

#define PIN_BACKLIGHT 9


#define BUTTON_A 3
#define BUTTON_B 7
#define BUTTON_C 11
#define BUTTON_D 15

#define BUTTON_POWER 12

enum TABS
{
    TAB_A,
    TAB_B,
    TAB_C,
    TAB_D,
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

//?##################################################################################
//*         Globals
uint_fast8_t currTab = TAB_A;
uint_fast8_t currMode = MODE_ON;

//?##################################################################################
//*         prototypes
void setBrightness(uint8_t brightness);
void handleScreen(void *args);
void handleButtons(void *args);
void mainSystem(void *args);

void debug(void *args);

#pragma once