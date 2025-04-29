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
    case MACROS_HOLD:
    case MACROS_HOLD_TOGGLE:
        if (status != previousStatus)
        {
            if (status)
            {
                log_i("macros  called, type: %i", macrosType);
                macrosItself();
            }
            else
            {
                log_i("macros stopped, type: %i", macrosType);
                macrosOnStop();
            }
        }
        previousStatus = status; // used as prevuousState
        break;
    case MACROS_ONCE:
        if (status)
        {
            log_i("macros  called, type: %i", macrosType);
            macrosItself();
            status = false;
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
    case MACROS_HOLD:
    case MACROS_CYCLIC:
        status = active;
        break;
    case MACROS_HOLD_TOGGLE:
    case MACROS_CYCLIC_TOGGLE:
    case MACROS_ONCE:
        status ^= active and !previousPoke;
        previousPoke = active;
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
