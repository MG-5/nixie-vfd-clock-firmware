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

    // all six digits (HH:MM:SS)
    void setClock(uint8_t hours,
                  uint8_t minutes,
                  uint8_t seconds)
    {
        this->hours = hours;
        this->minutes = minutes;
        this->seconds = seconds;
    }

protected:
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
};