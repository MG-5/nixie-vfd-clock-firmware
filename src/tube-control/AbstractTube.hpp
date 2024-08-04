#pragma once

#include "main.h"
#include "units/si/frequency.hpp"
#include "units/si/scalar.hpp"
#include "units/si/time.hpp"

class AbstractTube
{
public:
    static constexpr auto NumberOfTubes = 6;
    static constexpr auto NumberOfDigits = 10;

    // APB2 = 64MHz -> 1MHz = 1µs -> prescaler 64-1
    // auto reload period = 249 -> interrupt every 250µs
    static constexpr auto MultiplexingStepPeriod = 250.0_us;

    AbstractTube() = default;

    // initialization
    virtual void setup() = 0;

    // update step
    virtual void multiplexingStep(bool isFading = false) = 0;

    void enableDots(bool enable)
    {
        shouldDotsLights = enable;
    }

    virtual void shutdownCurrentTubeAndDot() = 0;

    constexpr virtual size_t getStepsPerFadingPeriod() = 0;

    virtual void prepareFadingDigit() = 0;
    virtual void updateFadingDigit() = 0;

    struct Clock_t
    {
        uint8_t hours = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
    };

    Clock_t currentClockTime;
    Clock_t prevClockTime;

protected:
    // init with last tube due
    // multiplexingStep() increment at start
    uint8_t tubeIndex = AbstractTube::NumberOfTubes - 1;

    uint8_t getDigitFromClockTime(const Clock_t &clockTime)
    {
        switch (tubeIndex)
        {
        case 0:
            return clockTime.hours / 10;

        case 1:
            return clockTime.hours % 10;

        case 2:
            return clockTime.minutes / 10;

        case 3:
            return clockTime.minutes % 10;

        case 4:
            return clockTime.seconds / 10;

        case 5:
            return clockTime.seconds % 10;

        default:
            return 0;
        }
    }

    bool shouldDotsLights = true;
};