#ifndef MACROS_H
#define MACROS_H

#include <stdint.h>

enum MacrosType
{
    MACROS_CYCLIC,
    MACROS_CYCLIC_TOGGLE,
    MACROS_ONCE,
    MACROS_HOLD,
    MACROS_HOLD_TOGGLE,
};

class Macros
{
private:
    MacrosType macrosType;
    bool previousPoke = false;
    bool status = false;
    int64_t timer = 0;
    int64_t period;
    void (*macroItself)();

public:
    Macros() = delete;
    Macros(void (*setMacroItself)(), MacrosType setTogglable, int64_t period);
    ~Macros();

    uint_fast8_t runMacro();
    uint_fast8_t pokeMacro(bool active);
    bool getStatus();
};

#include "Macros.cpp"
#endif // MACROS_H