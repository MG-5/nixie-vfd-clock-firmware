#include "Nixie.hpp"
#include "units/si/scalar.hpp"
#include "units/si/time.hpp"

void Nixie::setup()
{
    enableBoostConverter.write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::multiplexingStep()
{
    static uint8_t tubeIndex = 0;

    // turn off all tubes and digits
    for (auto &tube : tubeArray)
        tube.write(false);

    for (auto &digit : digitArray)
        digit.write(false);

    dots.write(false);
    leftComma.write(false);

    // start rejuvenating every minute
    if (clockTime.seconds == 0 && !isRejuvenating)
        isRejuvenating = true;

    tubeArray[tubeIndex].write(true);

    if (isRejuvenating)
        rejuvenateStep(tubeIndex);

    else
        displayTimeOnTubes(tubeIndex);

    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;
}

//--------------------------------------------------------------------------------------------------
void Nixie::rejuvenateStep(uint8_t tubeIndex)
{
    static uint8_t digit = 0;
    static uint16_t rejuvenationCounter = 0;

    // alls digits must be turned on for 200ms rejuvenation respectively
    constexpr auto RejuvenationTime = 200.0_ms;
    constexpr auto RejuvenationSteps =
        (RejuvenationTime / (1.0_ / MultiplexingFrequency)).getMagnitude<uint16_t>() /
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
void Nixie::displayTimeOnTubes(uint8_t tubeIndex)
{
    switch (tubeIndex)
    {
    case 0:
        digitArray[clockTime.hours / 10].write(true);
        break;

    case 1:
        digitArray[clockTime.hours % 10].write(true);
        break;

    case 2:
        digitArray[clockTime.minutes / 10].write(true);
        break;

    case 3:
        digitArray[clockTime.minutes % 10].write(true);
        break;

    case 4:
        digitArray[clockTime.seconds / 10].write(true);
        break;

    case 5:
        digitArray[clockTime.seconds % 10].write(true);
        break;

    default:
        break;
    }

    if (tubeIndex == 0 && shouldShowDots)
        dots.write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::enableDots(bool enable)
{
    shouldShowDots = enable;
}

//--------------------------------------------------------------------------------------------------
void Nixie::disableAllTubes()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    dots.write(false);
}