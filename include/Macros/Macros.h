#ifndef MACROS_H
#define MACROS_H

#include <stdint.h>

class macros
{
private:
    bool status;
    bool togglable;
    void (*macroItself)();
    uint64_t period;

public:
    macros(void (*setMacroItself)(), bool setTogglable, uint64_t period);
    ~macros();

    uint_fast8_t runMacro();
    uint_fast8_t pokeMacro(bool active);
    bool getStatus();
};

#include "Macros.cpp"
#endif // MACROS_H