#include "Nixie.hpp"

void Nixie::setup()
{
    enableBoostConverter.write(true);
    enableDots(true);

    // leftComma.write(true);
    // rightComma.write(true);
}

//--------------------------------------------------------------------------------------------------
void Nixie::multiplexingStep()
{
    static uint8_t tubeIndex = 0;

    for (auto &tube : tubeArray)
        tube.write(false);

    for (auto &digit : digitArray)
        digit.write(false);

    tubeArray[tubeIndex].write(true);

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

    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;
}

//--------------------------------------------------------------------------------------------------
void Nixie::enableDots(bool enable)
{
    dotsEnable.write(enable);
}

//--------------------------------------------------------------------------------------------------
void Nixie::disableAllTubes()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    // enableDots(false);
}