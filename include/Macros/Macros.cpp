#include "Macros.h"

/// @brief
/// @param setMacroItself function to run on tick
/// @param setTogglable   does "poke" toggle macros
/// @param setPeriod      uS period
Macros::Macros(void (*setMacroItself)(), bool setTogglable, int64_t setPeriod)
{
    macroItself = setMacroItself;
    togglable = setTogglable;
    period = setPeriod;
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
        status ^= active and !previousPoke;
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
