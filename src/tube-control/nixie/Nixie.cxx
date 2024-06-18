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
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    // turn off all tubes and digits
    disableAllTubes();

    for (auto &digit : digitArray)
        digit.write(false);

    leftComma.write(false);

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

    if (digit == 0)
        leftComma.write(true);

    digitArray[digit].write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::displayTimeOnTubes(bool isFading)
{
    uint8_t numberToShow = getDigitFromClockTime(isFading ? prevClockTime : currentClockTime);
    digitArray[numberToShow].write(true);

    if (tubeIndex == 0 && shouldDotsLights)
        dots.write(true);
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::disableAllTubes()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
inline void Nixie::updateFadingDigit()
{
    if (isRejuvenating)
        return;

    uint8_t oldNumber = getDigitFromClockTime(prevClockTime);
    uint8_t newNumberToShow = getDigitFromClockTime(currentClockTime);

    digitArray[oldNumber].write(false);
    digitArray[newNumberToShow].write(true);
}