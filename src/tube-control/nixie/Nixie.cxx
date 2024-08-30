#include "Nixie.hpp"
#include "helpers/freertos.hpp"
#include "task.h"
#include "units/si/scalar.hpp"
#include "units/si/time.hpp"

void Nixie::setup()
{
    setBoostConverterState(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::setBoostConverterState(bool enable)
{
    enableBoostConverter.write(enable);
}

//--------------------------------------------------------------------------------------------------
void Nixie::renderInitialization()
{
    for (auto &tube : tubeGpioArray)
    {
        tube.write(true);

        for (int digit = 0; digit < AbstractTube::NumberOfDigits; digit++)
        {
            digitGpioArray[digit].write(true);
            vTaskDelay(toOsTicks(150.0_ms));
            digitGpioArray[digit].write(false);
        }

        tube.write(false);
    }
}

//--------------------------------------------------------------------------------------------------
void Nixie::multiplexingStep(bool isFading)
{
    uint8_t prevTubeIndex = tubeIndex;
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    tubeGpioArray[prevTubeIndex].write(false);

    for (auto &digit : digitGpioArray)
        digit.write(false);

    // start rejuvenating every minute at 30. second
    if (digitDataArray2[4].digit == 3 && digitDataArray2[5].digit == 0 && !isRejuvenating)
        isRejuvenating = true;

    if (isRejuvenating)
        rejuvenateStep();

    else
    {
        uint8_t digitToShow =
            isFading ? digitDataArray1[tubeIndex].digit : digitDataArray2[tubeIndex].digit;
        uint8_t isCommaLeftEnabled =
            isFading ? digitDataArray1[tubeIndex].commaLeft : digitDataArray2[tubeIndex].commaLeft;

        if (clockArrivedOnce)
            digitGpioArray[digitToShow].write(true);
        leftComma.write(isCommaLeftEnabled);
    }

    tubeGpioArray[tubeIndex].write(true);

    dots.write(tubeIndex == 0 && shouldDotsLights);
}

// -------------------------------------------------------------------------------------------------
void Nixie::renderClock(Time &newClock)
{
    digitDataArray1 = digitDataArray2;

    clockArrivedOnce = true;
    for (auto i = 0; i < NumberOfTubes; i++)
    {
        digitDataArray2[i].digit = getDigitFromClockTime(newClock, i);
        digitDataArray2[i].commaLeft = false;
    }

    setDotState(newClock.second % 2 == 0);
}

//--------------------------------------------------------------------------------------------------
void Nixie::renderText(const std::string &text)
{
    // Nixie tubes do not support text rendering so turn off all tubes
    shutdownAllTubesAndDots();
}

//--------------------------------------------------------------------------------------------------
void Nixie::rejuvenateStep()
{
    static uint8_t digit = 0;
    static uint16_t rejuvenationCounter = 0;

    // alls digits must be turned on for 200ms rejuvenation respectively
    constexpr auto RejuvenationTime = 200.0_ms;
    constexpr auto RejuvenationSteps =
        (RejuvenationTime / MultiplexingStepPeriod).getMagnitude<uint16_t>() /
        AbstractTube::NumberOfTubes;

    if (tubeIndex == 0)
        if (++digit >= AbstractTube::NumberOfDigits)
        {
            digit = 0;

            if (++rejuvenationCounter >= RejuvenationSteps)
            {
                rejuvenationCounter = 0;
                isRejuvenating = false;
                return;
            }
        }

    digitGpioArray[digit].write(true);
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::shutdownCurrentTubeAndDot()
{
    tubeGpioArray[tubeIndex].write(false);
    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
void Nixie::shutdownAllTubesAndDots()
{
    for (auto &tube : tubeGpioArray)
        tube.write(false);

    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
void Nixie::prepareFadingDigit()
{
    newNumberToShow = digitDataArray2[tubeIndex].digit;
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::updateFadingDigit()
{
    if (isRejuvenating)
        return;

    for (auto &digit : digitGpioArray)
        digit.write(false);

    digitGpioArray[newNumberToShow].write(true);
}