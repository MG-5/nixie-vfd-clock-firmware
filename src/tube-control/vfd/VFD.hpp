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
    void shutdownAllTubesAndDots() override;

    void prepareFadingDigit() override;
    void updateFadingDigit() override;

    constexpr size_t getStepsPerFadingPeriod() override
    {
        return (160.0_ms / AbstractTube::MultiplexingStepPeriod).getMagnitude<size_t>();
    };

protected:
    void powerOn() override;
    void powerOff() override;
    void renderInitialization() override;

    void multiplexingStep(bool isFading) override;
    void renderClock(Time &newClock) override;
    void renderText(const std::string &text) override;

private:
    static constexpr auto NumberBitsInShiftRegister = 20;

    util::Gpio enableBoostConverter{Enable50V_190V_GPIO_Port, Enable50V_190V_Pin};
    util::Gpio heatwireEnable{Digit4_Heatwire_GPIO_Port, Digit4_Heatwire_Pin};
    util::Gpio dots{Dots_GPIO_Port, Dots_Pin};

    std::array<util::Gpio, NumberOfTubes> gridGpioArray{util::Gpio{Tube0_GPIO_Port, Tube0_Pin}, //
                                                        util::Gpio{Tube1_GPIO_Port, Tube1_Pin}, //
                                                        util::Gpio{Tube2_GPIO_Port, Tube2_Pin}, //
                                                        util::Gpio{Tube3_GPIO_Port, Tube3_Pin}, //
                                                        util::Gpio{Tube4_GPIO_Port, Tube4_Pin}, //
                                                        util::Gpio{Tube5_GPIO_Port, Tube5_Pin}};

    util::Gpio strobe{Digit3_Strobe_GPIO_Port, Digit3_Strobe_Pin};

    util::Gpio shiftRegisterData{Digit9_SR_Data_GPIO_Port, Digit9_SR_Data_Pin};
    util::Gpio shiftRegisterClock{Digit8_SR_CLK_GPIO_Port, Digit8_SR_CLK_Pin};

    void clockPeriod();
    void strobePeriod();
    void sendSegmentBits(uint32_t bits, uint8_t commatas = 0);

    struct GridData
    {
        uint16_t segments = 0;
        uint8_t commatas = 0b11;
    };

    std::array<GridData, NumberOfTubes> gridDataArray1{};
    std::array<GridData, NumberOfTubes> gridDataArray2{};
};
