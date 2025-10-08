#pragma once

#include <array>

#include "../BaseTubeDisplay.hpp"
#include "main.h"
#include "util/gpio.hpp"

class Nixie : public BaseTubeDisplay
{
public:
    Nixie() = default;

protected:
    void powerOn() override;
    void powerOff() override;
    void renderInitialization() override;

    void multiplexingStep(bool isFading, bool showSeconds) override;
    void renderClock(Time &newClock) override;
    void renderText(const std::string &text) override;
    void setDigit(uint8_t number, uint8_t index) override;

    void shutdownCurrentTubeAndDot() override;
    void shutdownAllTubesAndDots() override;
    void prepareFadingDigit() override;
    void updateFadingDigit() override;

private:
    util::Gpio enableBoostConverter{Enable50V_190V_GPIO_Port, Enable50V_190V_Pin};
    util::Gpio dots{Dots_GPIO_Port, Dots_Pin};
    util::Gpio leftComma{LeftComma_GPIO_Port, LeftComma_Pin};
    util::Gpio rightComma{RightComma_GPIO_Port, RightComma_Pin};

    bool isRejuvenating = false;

    uint8_t newNumberToShow = 0;

    void rejuvenateStep();

    // GPIOs for tube selection
    std::array<util::Gpio, NumberOfTubes> tubeGpioArray{
        util::Gpio{Tube0_GPIO_Port, Tube0_Pin}, //
        util::Gpio{Tube1_GPIO_Port, Tube1_Pin}, //
        util::Gpio{Tube2_GPIO_Port, Tube2_Pin}, //
        util::Gpio{Tube3_GPIO_Port, Tube3_Pin}, //
        util::Gpio{Tube4_GPIO_Port, Tube4_Pin}, //
        util::Gpio{Tube5_GPIO_Port, Tube5_Pin}  //
    };

    // GPIOs for digit selection
    std::array<util::Gpio, NumberOfDigits> digitGpioArray{
        util::Gpio{Digit0_GPIO_Port, Digit0_Pin},                   //
        util::Gpio{Digit1_GPIO_Port, Digit1_Pin},                   //
        util::Gpio{Digit2_Blanking_GPIO_Port, Digit2_Blanking_Pin}, //
        util::Gpio{Digit3_Strobe_GPIO_Port, Digit3_Strobe_Pin},     //
        util::Gpio{Digit4_Heatwire_GPIO_Port, Digit4_Heatwire_Pin}, //
        util::Gpio{Digit5_GPIO_Port, Digit5_Pin},                   //
        util::Gpio{Digit6_GPIO_Port, Digit6_Pin},                   //
        util::Gpio{Digit7_GPIO_Port, Digit7_Pin},                   //
        util::Gpio{Digit8_SR_CLK_GPIO_Port, Digit8_SR_CLK_Pin},     //
        util::Gpio{Digit9_SR_Data_GPIO_Port, Digit9_SR_Data_Pin}    //
    };

    struct DigitValue
    {
        uint8_t digit = 0;
        bool commaLeft = true;
    };

    // store two arrays for fading
    std::array<DigitValue, NumberOfTubes> currentDigitValues{};
    std::array<DigitValue, NumberOfTubes> targetDigitValues{};

    bool clockArrivedOnce = false;
};
