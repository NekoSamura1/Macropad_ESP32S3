#include "Macros.h"

Macros::Macros(void (*setMacrosItself)(), void (*setMacrosOnStop)(), MacrosType setMacrosType, int64_t setPeriod)
{
    macrosItself = setMacrosItself;
    macrosOnStop = setMacrosOnStop;
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
            macrosItself();
        }
        break;
    default:
        return 1;
        break;
    }
    return 0;
}

uint_fast8_t Macros::pokeMacro(const bool active)
{
    switch (macrosType)
    {
    case MACROS_CYCLIC:
        status = active;
        break;
    case MACROS_CYCLIC_TOGGLE:
        status ^= active and !previous;
        previous = active;
        break;
    case MACROS_HOLD:
        status = active;
        if (status!=previous){
            if (status)
            {
                macrosItself();
            }
            else
            {
                macrosOnStop();
            }
        }
        previous = status; // used as prevuousState
        break;
    case MACROS_HOLD_TOGGLE:
        status ^= active and !previous;
        if (active and !previous)
        {
            if (status)
            {
                macrosItself();
            }
            else
            {
                macrosOnStop();
            }
        }
        previous = active;
        break;
    default:
        return 3; // error wrong mode
        break;
    }

    return uint_fast8_t(status);
}

bool Macros::getStatus()
{
    return status;
}
