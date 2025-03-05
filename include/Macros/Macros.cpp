#include "Macros.h"

/// @brief
/// @param setMacroItself function to run on tick
/// @param setMacrosType   does "poke" toggle macros
/// @param setPeriod      uS period
Macros::Macros(void (*setMacroItself)(), MacrosType setMacrosType, int64_t setPeriod)
{
    macroItself = setMacroItself;
    macrosType = setMacrosType;
    period = setPeriod;
}

Macros::~Macros()
{
}

uint_fast8_t Macros::runMacro()
{
    switch (macrosType)
    {
    case MACROS_CYCLIC:
    case MACROS_CYCLIC_TOGGLE:
        if (status and esp_timer_get_time() - timer > period)
        {
            timer = esp_timer_get_time();
            macroItself();
        }
        break;
    default:
        break;
    }
    return uint_fast8_t();
}

uint_fast8_t Macros::pokeMacro(bool active)
{
    switch (macrosType)
    {
    case MACROS_CYCLIC:
        status = active;
        break;
    case MACROS_CYCLIC_TOGGLE:
        status ^= active and !previousPoke;
        break;
    case MACROS_HOLD_TOGGLE:
    default:
    return 3; //error wrong mode
        break;
    }

    return uint_fast8_t(status);
}

bool Macros::getStatus()
{
    return status;
}
