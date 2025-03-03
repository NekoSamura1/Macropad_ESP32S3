#include "macros.h"

macros::macros(void (*setMacroItself)(), bool setTogglable, uint64_t setPeriod)
{
    macroItself = setMacroItself;
    togglable = setTogglable;
    period = setPeriod;
    status = false;
}

macros::~macros()
{
}

uint_fast8_t macros::runMacro()
{
    if (status)
    {
        macroItself();
    }
    return uint_fast8_t();
}

uint_fast8_t macros::pokeMacro(bool active)
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

bool macros::getStatus()
{
return status;
}
