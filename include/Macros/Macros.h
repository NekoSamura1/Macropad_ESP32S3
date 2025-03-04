#ifndef MACROS_H
#define MACROS_H

#include <stdint.h>

class Macros
{
private:
    bool previousPoke = false;
    bool status = false;
    bool togglable;
    void (*macroItself)();
    int64_t period;

public:
    Macros() = delete;
    Macros(void (*setMacroItself)(), bool setTogglable, int64_t period);
    ~Macros();

    uint_fast8_t runMacro();
    uint_fast8_t pokeMacro(bool active);
    bool getStatus();
};

#include "Macros.cpp"
#endif // MACROS_H