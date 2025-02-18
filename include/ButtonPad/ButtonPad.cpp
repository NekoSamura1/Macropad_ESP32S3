#include "ButtonPad.h"

void buttonStateInit()
{
    pinMode(COLUMN1, INPUT_PULLUP);
    pinMode(COLUMN2, INPUT_PULLUP);
    pinMode(COLUMN3, INPUT_PULLUP);
    pinMode(COLUMN4, INPUT_PULLUP);

    digitalWrite(ROW1, 0);
    digitalWrite(ROW2, 0);
    digitalWrite(ROW3, 0);
    digitalWrite(ROW4, 0);
}

void buttonStateUpdate(uint16_t *state)
{
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