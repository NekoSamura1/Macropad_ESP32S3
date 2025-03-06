#ifndef MACROS_H
#define MACROS_H

#include <stdint.h>
#include <esp_timer.h>

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
    void (*macrosItself)();
    void (*macrosOnStop)() = nullptr; //used only in HOLD modes
    int64_t timer = 0;
    int64_t period;
    bool previousStatus = false;
    bool previousPoke = false;
    bool status = false;

public:
    Macros() = delete;

    /// @brief
    /// @param setMacrosItself function to run on tick
    /// @param setMacrosOnStop used in HOLD modes to unpress pressed keys
    /// @param setMacrosType   does "poke" toggle macros
    /// @param setPeriod      uS period
    Macros(void (*setMacrosItself)(), void (*setMacrosOnStop)(), MacrosType setMacrosType, int64_t setPeriod);
    ~Macros();

    uint_fast8_t runMacro();
    uint_fast8_t pokeMacro(const bool active);
    bool getStatus();
};

// #include "Macros.cpp"
#endif // MACROS_H