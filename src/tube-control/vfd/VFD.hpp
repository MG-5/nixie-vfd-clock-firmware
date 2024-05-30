#pragma once

#include <array>

#include "../AbstractTube.hpp"
#include "DelayTimer.hpp"
#include "main.h"
#include "util/gpio.hpp"

class VFD : public AbstractTube
{
public:
    VFD(TIM_HandleTypeDef *delayTimerHandle) : delayTimer(delayTimerHandle){};

    // void notifySpiIsFinished();
    void disableAllTubes() override;

protected:
    void setup() override;
    void multiplexingStep() override;

    void enableDots(bool enable) override;

private:
    // SPI_HandleTypeDef *spiPeripherie = nullptr;

    static constexpr auto NumberBitsInShiftRegister = 20;
    bool shouldDotsLights = false;

    util::Gpio enableBoostConverter{Enable50V_190V_GPIO_Port, Enable50V_190V_Pin};
    util::Gpio heatwireEnable{Digit4_Heatwire_GPIO_Port, Digit4_Heatwire_Pin};
    util::Gpio dotsEnable{Dots_GPIO_Port, Dots_Pin};

    std::array<util::Gpio, 6> tubeArray{util::Gpio{Tube0_GPIO_Port, Tube0_Pin}, //
                                        util::Gpio{Tube1_GPIO_Port, Tube1_Pin}, //
                                        util::Gpio{Tube2_GPIO_Port, Tube2_Pin}, //
                                        util::Gpio{Tube3_GPIO_Port, Tube3_Pin}, //
                                        util::Gpio{Tube4_GPIO_Port, Tube4_Pin}, //
                                        util::Gpio{Tube5_GPIO_Port, Tube5_Pin}};

    util::Gpio strobe{Digit3_Strobe_GPIO_Port, Digit3_Strobe_Pin};

    void clockPeriod();
    void strobePeriod();
    void sendSegmentBits(uint32_t bits);

    static constexpr std::array<uint32_t, 10> numberSegments{
        0b1111111100001001, // 0
        0b0000000000010010, // 1
        0b1110111011000000, // 2
        0b1111110001000000, // 3
        0b0011000111000000, // 4
        0b1101110111000000, // 5
        0b1101111111000000, // 6
        0b1111000000000000, // 7
        0b1111111111000000, // 8
        0b1111110111000000  // 9
    };

    DelayTimer delayTimer;

    util::Gpio shiftRegisterData{Digit9_SR_Data_GPIO_Port, Digit9_SR_Data_Pin};
    util::Gpio shiftRegisterClock{Digit8_SR_CLK_GPIO_Port, Digit8_SR_CLK_Pin};
};
