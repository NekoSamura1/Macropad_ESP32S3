#ifndef BUTTONPAD_H
#define BUTTONPAD_H

#include <Arduino.h>
#include <stdint.h>
#ifdef COMPACT_DEV
#define COLUMN1 17
#define COLUMN2 15
#define COLUMN3 16
#define COLUMN4 18

#define ROW1 6
#define ROW2 7
#define ROW3 5
#define ROW4 4
#else
#define COLUMN1 18
#define COLUMN2 17
#define COLUMN3 16
#define COLUMN4 15

#define ROW1 7
#define ROW2 6
#define ROW3 5
#define ROW4 4
#endif

#define DEBOUNCE_TIME 20     // 20 мс
#define LONG_PRESS_TIME 1000 // 1 секунда

typedef enum {
    BTN_STATE_RELEASED,   // Кнопка отпущена
    BTN_STATE_PRESSED,    // Нажата
    BTN_STATE_LONG_PRESS, // Долгое нажатие
    BTN_STATE_DEBOUNCE,   // Дребезг
} BtnState;

typedef struct Button_t {
    bool wasPressed;
    bool wasReleased;
    uint8_t buttNumber; // 0-16
    BtnState state;
    uint32_t last_time;
} Button_t;

/// @brief initialize pin modes for the button pad
void buttonStateInit();

/// @brief writes the state of the buttons in bit form to the pointer
/// @param state pointer to the variable where the state will be written in bit form
void buttonStateUpdate(uint16_t* state);

// #include "ButtonPad.cpp"
#endif // BUTTONPAD_H