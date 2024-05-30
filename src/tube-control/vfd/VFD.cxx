#include "VFD.hpp"

#include "helpers/freertos.hpp"
#include "task.h"

void VFD::setup()
{
    // enableBoostConverter.write(true);
    delayTimer.startTimer();

    heatwireEnable.write(true);
    vTaskDelay(toOsTicks(100.0_ms));

    shiftRegisterClock.write(false);
    shiftRegisterData.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::multiplexingStep()
{
    static uint8_t tubeIndex = 0;

    disableAllTubes();

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
inline void VFD::disableAllTubes()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    dotsEnable.write(false); // directly disable dot anodes
}

//--------------------------------------------------------------------------------------------------
void VFD::enableDots(bool enable)
{
    shouldDotsLights = enable;
}

//--------------------------------------------------------------------------------------------------
void VFD::clockPeriod()
{
    shiftRegisterClock.write(true);
    delayTimer.delay187ns();
    shiftRegisterClock.write(false);
    delayTimer.delay187ns();
}

//--------------------------------------------------------------------------------------------------
void VFD::strobePeriod()
{
    strobe.write(true);
    delayTimer.delay187ns();
    strobe.write(false);
    delayTimer.delay187ns();
}

//--------------------------------------------------------------------------------------------------
void VFD::sendSegmentBits(uint32_t bits)
{
    bits <<= 4;        // make place for dots and commats bits
    bits |= 0b11;      // dots are always enabled - will be switched by anode driver
    bits &= ~(0b1100); // commatas are currenty disabled

    for (auto i = 0; i < NumberBitsInShiftRegister; i++)
    {
        shiftRegisterData.write((bits >> i) & 1);
        clockPeriod();
    }

    strobePeriod();
}