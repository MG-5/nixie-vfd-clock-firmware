#include "VFD.hpp"

#include "font/Font.hpp"
#include "helpers/freertos.hpp"
#include "task.h"
#include "units/si/time.hpp"
#include "util/MapValue.hpp"

void VFD::powerOn()
{
    enableBoostConverter.write(true);
    heatwireEnable.write(true);
}

//--------------------------------------------------------------------------------------------------
void VFD::powerOff()
{
    enableBoostConverter.write(false);
    heatwireEnable.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::multiplexingStep(bool isFading)
{
    uint8_t prevTubeIndex = tubeIndex;
    if (++tubeIndex >= AbstractTube::NumberOfTubes)
        tubeIndex = 0;

    sendSegmentBits(
        isFading ? gridDataArray1[tubeIndex].segments : gridDataArray2[tubeIndex].segments,
        isFading ? gridDataArray1[tubeIndex].commatas : gridDataArray2[tubeIndex].commatas);

    gridGpioArray[prevTubeIndex].write(false);
    strobePeriod();

    gridGpioArray[tubeIndex].write(true);
    dots.write(tubeIndex == 0 && shouldDotsLights);
}

//--------------------------------------------------------------------------------------------------
void VFD::renderInitialization()
{
    dots.write(false);
    for (auto &grid : gridGpioArray)
        grid.write(true);

    uint32_t bits = 1;

    while ((bits & (1 << (NumberBitsInShiftRegister - 4))) == 0)
    {
        sendSegmentBits(bits);
        strobePeriod();
        vTaskDelay(toOsTicks(100.0_ms));

        bits <<= 1;
    }

    bits >>= 1;
    bits |= 1 << (NumberBitsInShiftRegister - 5);

    while ((bits & 1) == 0)
    {
        bits >>= 1;
        bits |= 1 << (NumberBitsInShiftRegister - 5);

        sendSegmentBits(bits);
        strobePeriod();
        vTaskDelay(toOsTicks(100.0_ms));
    }

    sendSegmentBits(bits);
    strobePeriod();
    vTaskDelay(toOsTicks(100.0_ms));
    sendSegmentBits(bits, 0b11);
    dots.write(true);
    strobePeriod();
}

// -------------------------------------------------------------------------------------------------
void VFD::renderClock(Time &newClock)
{
    gridDataArray1 = gridDataArray2;

    for (auto i = 0; i < NumberOfTubes; i++)
    {
        gridDataArray2[i].segments = font.getGlyph(getDigitFromClockTime(newClock, i) + '0');
        gridDataArray2[i].commatas = 0;
    }

    shouldDotsLights = newClock.second % 2 == 0;
}

//--------------------------------------------------------------------------------------------------
void VFD::renderText(const std::string &text)
{
    gridDataArray1 = gridDataArray2;

    for (auto i = 0; i < NumberOfTubes; i++)
    {
        gridDataArray2[i].segments = font.getGlyph(text[i]);
        gridDataArray2[i].commatas = 0;
    }

    shouldDotsLights = false;
}

//--------------------------------------------------------------------------------------------------
inline void VFD::shutdownCurrentTubeAndDot()
{
    gridGpioArray[tubeIndex].write(false);
    dots.write(false); // directly disable dot anodes
}

//--------------------------------------------------------------------------------------------------
void VFD::shutdownAllTubesAndDots()
{
    for (auto &gridGpio : gridGpioArray)
        gridGpio.write(false);

    dots.write(false);
}

//--------------------------------------------------------------------------------------------------
void VFD::prepareFadingDigit()
{
    // write next digit to shift register without latching
    sendSegmentBits(gridDataArray2[tubeIndex].segments, gridDataArray2[tubeIndex].commatas);
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
void VFD::sendSegmentBits(uint32_t bits, uint8_t commatas)
{
    bits <<= 4;                     // shift bits to make place for dots and commats bits
    bits |= 0b11;                   // dots are always enabled - will be switched by anode driver
    bits |= (commatas & 0b11) << 2; // commatas

    for (auto i = 0; i < NumberBitsInShiftRegister; i++)
    {
        shiftRegisterData.write((bits >> i) & 1);
        clockPeriod();
    }
}