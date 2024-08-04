#include "Nixie.hpp"
#include "units/si/scalar.hpp"
#include "units/si/time.hpp"

void Nixie::setup()
{
    enableBoostConverter.write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::multiplexingStep(bool isFading)
{
    uint8_t prevTubeIndex = tubeIndex;
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    tubeArray[prevTubeIndex].write(false);

    for (auto &digit : digitArray)
        digit.write(false);

    // start rejuvenating every minute
    if (currentClockTime.seconds == 30 && !isRejuvenating)
        isRejuvenating = true;

    if (isRejuvenating)
        rejuvenateStep();

    else
        displayTimeOnTubes(isFading);

    tubeArray[tubeIndex].write(true);
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

    digitArray[digit].write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::displayTimeOnTubes(bool isFading)
{
    uint8_t numberToShow = getDigitFromClockTime(isFading ? prevClockTime : currentClockTime);
    digitArray[numberToShow].write(true);

    dots.write(tubeIndex == 0 && shouldDotsLights);
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::shutdownCurrentTubeAndDot()
{
    tubeArray[tubeIndex].write(false);
    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
void Nixie::prepareFadingDigit()
{
    oldNumber = getDigitFromClockTime(prevClockTime);
    newNumberToShow = getDigitFromClockTime(currentClockTime);
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::updateFadingDigit()
{
    if (isRejuvenating)
        return;

    digitArray[oldNumber].write(false);
    digitArray[newNumberToShow].write(true);
}