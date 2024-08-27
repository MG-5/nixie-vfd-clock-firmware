#include "VFD.hpp"

#include "font/Font.hpp"
#include "helpers/freertos.hpp"
#include "task.h"
#include "units/si/time.hpp"
#include "util/MapValue.hpp"

void VFD::setup()
{
    setBoostConverterState(true);
}

//--------------------------------------------------------------------------------------------------
void VFD::setBoostConverterState(bool enable)
{
    enableBoostConverter.write(enable);
    heatwireEnable.write(enable);
}

//--------------------------------------------------------------------------------------------------
void VFD::multiplexingStep(bool isFading)
{
    uint8_t prevTubeIndex = tubeIndex;
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    uint8_t numberToShow = getDigitFromClockTime(isFading ? prevClockTime : currentClockTime);

    sendSegmentBits(font.getGlyph(numberToShow + '0'));
    tubeArray[prevTubeIndex].write(false);
    strobePeriod();

    tubeArray[tubeIndex].write(true);
    dots.write(tubeIndex == 0 && shouldDotsLights);
}

//--------------------------------------------------------------------------------------------------
inline void VFD::shutdownCurrentTubeAndDot()
{
    tubeArray[tubeIndex].write(false);
    dots.write(false); // directly disable dot anodes
}

//--------------------------------------------------------------------------------------------------
void VFD::shutdownAllTubesAndDots()
{
    for (auto &tube : tubeArray)
        tube.write(false);

    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::prepareFadingDigit()
{
    // write next digit to shift register without latching
    uint8_t numberToShow = getDigitFromClockTime(currentClockTime);
    sendSegmentBits(font.getGlyph(numberToShow + '0'));
}

//--------------------------------------------------------------------------------------------------
inline void VFD::updateFadingDigit()
{
    // latch prepared shift register data
    strobePeriod();
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
}