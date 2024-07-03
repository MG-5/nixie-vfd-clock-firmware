#include "VFD.hpp"

#include "helpers/freertos.hpp"
#include "task.h"
#include "units/si/time.hpp"
#include "util/MapValue.hpp"

void VFD::setup()
{
    // enableBoostConverter.write(true);
    heatwireEnable.write(true);
}

//--------------------------------------------------------------------------------------------------
void VFD::multiplexingStep(bool isFading)
{
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    uint8_t numberToShow = getDigitFromClockTime(isFading ? prevClockTime : currentClockTime);

    disableAllTubes();
    sendSegmentBits(numberSegments[numberToShow]);

    tubeArray[tubeIndex].write(true);
    if (tubeIndex == 0 && shouldDotsLights)
        dots.write(true);
}

//--------------------------------------------------------------------------------------------------
inline void VFD::disableAllTubes()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    dots.write(false); // directly disable dot anodes
}

//--------------------------------------------------------------------------------------------------
inline void VFD::updateFadingDigit()
{
    uint8_t numberToShow = getDigitFromClockTime(currentClockTime);
    sendSegmentBits(numberSegments[numberToShow]);
}

//--------------------------------------------------------------------------------------------------
void VFD::clockPeriod()
{
    shiftRegisterClock.write(true);
    shiftRegisterClock.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::strobePeriod()
{
    strobe.write(true);
    strobe.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::sendSegmentBits(uint32_t bits)
{
    bits <<= 4;        // shift bits to make place for dots and commats bits
    bits |= 0b11;      // dots are always enabled - will be switched by anode driver
    bits &= ~(0b1100); // commatas are currenty disabled

    for (auto i = 0; i < NumberBitsInShiftRegister; i++)
    {
        shiftRegisterData.write((bits >> i) & 1);
        clockPeriod();
    }

    strobePeriod();
}