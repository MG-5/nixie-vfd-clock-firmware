#pragma once

#include <array>

#include "../AbstractTube.hpp"
#include "main.h"
#include "util/gpio.hpp"

class VFD : public AbstractTube
{
public:
    VFD() {};

    void shutdownCurrentTubeAndDot() override;

    void prepareFadingDigit() override;
    void updateFadingDigit() override;

    constexpr size_t getStepsPerFadingPeriod() override
    {
        return (120.0_ms / AbstractTube::MultiplexingStepPeriod).getMagnitude<size_t>();
    };

protected:
    void setup() override;
    void multiplexingStep(bool isFading) override;

private:
    static constexpr auto NumberBitsInShiftRegister = 20;

    util::Gpio enableBoostConverter{Enable50V_190V_GPIO_Port, Enable50V_190V_Pin};
    util::Gpio heatwireEnable{Digit4_Heatwire_GPIO_Port, Digit4_Heatwire_Pin};
    util::Gpio dots{Dots_GPIO_Port, Dots_Pin};

    std::array<util::Gpio, 6> tubeArray{util::Gpio{Tube0_GPIO_Port, Tube0_Pin}, //
                                        util::Gpio{Tube1_GPIO_Port, Tube1_Pin}, //
                                        util::Gpio{Tube2_GPIO_Port, Tube2_Pin}, //
                                        util::Gpio{Tube3_GPIO_Port, Tube3_Pin}, //
                                        util::Gpio{Tube4_GPIO_Port, Tube4_Pin}, //
                                        util::Gpio{Tube5_GPIO_Port, Tube5_Pin}};

    util::Gpio strobe{Digit3_Strobe_GPIO_Port, Digit3_Strobe_Pin};

    /*   A1   A2
        ----I ----
       | \  |  / |
      F|  H | J  |B
       |   \|/   |
     G1 ---- ---- G2
       |   /|\   |
      E|  M | K  |C
       | /  |  \ |
     D2 ----L---- D1 */
    static constexpr std::array<uint32_t, AbstractTube::NumberOfDigits> numberSegments{
        0b1111111100001001, // 0
        0b1000110000010010, // 1
        0b1110111011000000, // 2
        0b1111110001000000, // 3
        0b0011000111000000, // 4
        0b1101110111000000, // 5
        0b1101111111000000, // 6
        0b1111000000000000, // 7
        0b1111111111000000, // 8
        0b1111110111000000  // 9
    };

    util::Gpio shiftRegisterData{Digit9_SR_Data_GPIO_Port, Digit9_SR_Data_Pin};
    util::Gpio shiftRegisterClock{Digit8_SR_CLK_GPIO_Port, Digit8_SR_CLK_Pin};

    void clockPeriod();
    void strobePeriod();
    void sendSegmentBits(uint32_t bits);
};
