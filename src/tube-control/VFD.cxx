#include "VFD.hpp"

#include "helpers/freertos.hpp"
#include "task.h"

void VFD::setup()
{
    // enableBoostConverter.write(true);

    heatwireEnable.write(true);
    vTaskDelay(toOsTicks(100.0_ms));

    spiClock.write(false);
    spiData.write(false);
    enableBus.write(true);
}

//--------------------------------------------------------------------------------------------------
void VFD::multiplexingStep()
{
    static uint8_t tubeIndex = 0;

    for (auto &tube : tubeArray)
        tube.write(false);
    dotsEnable.write(false);

    uint8_t numberToShow = 0;

    switch (tubeIndex)
    {
    case 0:
        numberToShow = hours / 10;
        break;

    case 1:
        numberToShow = hours % 10;
        break;

    case 2:
        numberToShow = minutes / 10;
        break;

    case 3:
        numberToShow = minutes % 10;
        break;

    case 4:
        numberToShow = seconds / 10;
        break;

    case 5:
        numberToShow = seconds % 10;
        break;

    default:
        break;
    }

    sendSegmentBits(numberSegments[numberToShow]);

    tubeArray[tubeIndex].write(true);
    if (tubeIndex == 0 && shouldDotsLights)
        dotsEnable.write(true);

    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;
}

//--------------------------------------------------------------------------------------------------
void VFD::enableDots(bool enable)
{
    shouldDotsLights = enable;
}

//--------------------------------------------------------------------------------------------------
void VFD::sendSegmentBits(uint32_t bits)
{
    for (auto i = 0; i < 2; i++)
    {
        spiData.write(true);

        spiClock.write(true);
        delay390ns();
        spiClock.write(false);
        delay390ns();
    }

    for (auto i = 0; i < 2; i++)
    {
        spiData.write(false);

        spiClock.write(true);
        delay390ns();
        spiClock.write(false);
        delay390ns();
    }

    for (auto i = 0; i < 16; i++)
    {
        spiData.write((bits >> i) & 1);

        spiClock.write(true);
        delay390ns();
        spiClock.write(false);
        delay390ns();
    }

    strobe.write(true);
    delay390ns();
    strobe.write(false);
    delay390ns();
}

//--------------------------------------------------------------------------------------------------
void VFD::delay390ns()
{
    for (size_t i = 0; i < 50; i++)
    {
        asm("nop");
    }
}