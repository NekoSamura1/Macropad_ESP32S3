#include "ButtonPad.h"

void buttonStateInit() {
    pinMode(COLUMN1, INPUT_PULLUP);
    pinMode(COLUMN2, INPUT_PULLUP);
    pinMode(COLUMN3, INPUT_PULLUP);
    pinMode(COLUMN4, INPUT_PULLUP);

    digitalWrite(ROW1, 0);
    digitalWrite(ROW2, 0);
    digitalWrite(ROW3, 0);
    digitalWrite(ROW4, 0);
}

void buttonStateUpdate(uint16_t* state) {
    *state = 0;
    pinMode(ROW1, OUTPUT);
    if (!digitalRead(COLUMN1))
        *state |= 1 << 0;
    if (!digitalRead(COLUMN2))
        *state |= 1 << 1;
    if (!digitalRead(COLUMN3))
        *state |= 1 << 2;
    if (!digitalRead(COLUMN4))
        *state |= 1 << 3;
    pinMode(ROW1, INPUT);

    pinMode(ROW2, OUTPUT);
    if (!digitalRead(COLUMN1))
        *state |= 1 << 4;
    if (!digitalRead(COLUMN2))
        *state |= 1 << 5;
    if (!digitalRead(COLUMN3))
        *state |= 1 << 6;
    if (!digitalRead(COLUMN4))
        *state |= 1 << 7;
    pinMode(ROW2, INPUT);

    pinMode(ROW3, OUTPUT);
    if (!digitalRead(COLUMN1))
        *state |= 1 << 8;
    if (!digitalRead(COLUMN2))
        *state |= 1 << 9;
    if (!digitalRead(COLUMN3))
        *state |= 1 << 10;
    if (!digitalRead(COLUMN4))
        *state |= 1 << 11;
    pinMode(ROW3, INPUT);

    pinMode(ROW4, OUTPUT);
    if (!digitalRead(COLUMN1))
        *state |= 1 << 12;
    if (!digitalRead(COLUMN2))
        *state |= 1 << 13;
    if (!digitalRead(COLUMN3))
        *state |= 1 << 14;
    if (!digitalRead(COLUMN4))
        *state |= 1 << 15;
    pinMode(ROW4, INPUT);
}

/// @brief Process button state changes.
/// @param btn Pointer to the button structure.
void buttonProcess(Button_t* btn, uint16_t state) {
    bool current_state = state & 1 << btn->buttNumber;

    switch (btn->state) {
    case BTN_STATE_RELEASED:
        if (current_state) {
            btn->state = BTN_STATE_DEBOUNCE;
            btn->last_time = millis();
        }
        break;

    case BTN_STATE_DEBOUNCE:
        if (millis() - btn->last_time >= DEBOUNCE_TIME) {
            if (current_state) {
                btn->state = BTN_STATE_PRESSED;
                btn->wasPressed = 1; // Флаг нажатия
                // Здесь можно выполнить действие по нажатию или присрать колбэк
                // LOGF("Buton: %d, pressed\r\n", mylog2(btn->pin));

            } else {
                btn->state = BTN_STATE_RELEASED;
                btn->wasReleased = 1; // Флаг отпускания
                // LOGF("Buton: %d, released\r\n", mylog2(btn->pin));
            }
        }
        break;

    case BTN_STATE_PRESSED:
        if (!current_state) {
            btn->state = BTN_STATE_DEBOUNCE;
        } else if (millis() - btn->last_time >= LONG_PRESS_TIME) {
            btn->state = BTN_STATE_LONG_PRESS;
            // Обработка длинного нажатия
        }
        break;

    case BTN_STATE_LONG_PRESS:
        if (!current_state) {
            btn->state = BTN_STATE_DEBOUNCE;
        }
        break;
    }
}