#pragma once

#include "main.h"

class AbstractTube
{
public:
    static constexpr auto NumberOfTubes = 6;
    static constexpr auto NumberOfDigits = 10;

    AbstractTube() = default;

    // initialization
    virtual void setup() = 0;

    // update step
    virtual void multiplexingStep() = 0;

    virtual void enableDots(bool enable) = 0;

    virtual void disableAllTubes() = 0;

    struct Clock_t
    {
        uint8_t hours = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
    };

    // all six digits (HH:MM:SS)
    void setClock(Clock_t clockTime)
    {
        this->clockTime = clockTime;
    }

protected:
    Clock_t clockTime;
};