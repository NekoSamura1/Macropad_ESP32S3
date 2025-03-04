#include "Macros.h"

Macros::Macros(void (*setMacroItself)(), bool setTogglable, int64_t setPeriod)
{
    macroItself = setMacroItself;
    togglable = setTogglable;
    period = setPeriod;
    status = false;
}

Macros::~Macros()
{
}

uint_fast8_t Macros::runMacro()
{
    if (status)
    {
        macroItself();
    }
    return uint_fast8_t();
}

uint_fast8_t Macros::pokeMacro(bool active)
{
    if (togglable)
    {
        status ^= active;
    }
    else
    {
        status = active;
    }
    return uint_fast8_t(status);
}

bool Macros::getStatus()
{
return status;
}
