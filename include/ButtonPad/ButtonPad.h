#ifndef BUTTONPAD_H
#define BUTTONPAD_H

#include <stdint.h>
#include <Arduino.h>

#define COLUMN1 18
#define COLUMN2 17
#define COLUMN3 16
#define COLUMN4 15

#define ROW1 7
#define ROW2 6
#define ROW3 5
#define ROW4 4

void buttonStateInit();
void buttonStateUpdate(uint16_t *state);
#include "ButtonPad.cpp"
#endif // BUTTONPAD_H