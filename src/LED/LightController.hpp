#pragma once

#include "AddressableLedDriver.hpp"

#include "animations/LedAnimationBase.hpp"
#include "animations/RainbowAnimation.hpp"

#include "main.h"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"
#include <array>

/// Apply colors/animations to the addressable LEDs
class LightController : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    explicit LightController(SPI_HandleTypeDef *SpiDevice)
        : TaskWithMemberFunctionBase("lightControllerTask", 512, osPriorityLow4),
          ledDriver(SpiDevice) //
    {};

    ~LightController() override = default;

    void notifySpiIsFinished();

    enum class AnimationType
    {
        Off,
        Rainbow,
        SolidColor
    };

    AnimationType currentAnimation{AnimationType::Rainbow};
    AnimationType prevAnimation{AnimationType::Rainbow};

    void updateAnimationType(AnimationType newAnimationType)
    {
        if (currentAnimation != AnimationType::Off)
            prevAnimation = currentAnimation; // save last active state

        currentAnimation = newAnimationType;
        rainbowAnimation.resetAnimation();
    }

    void setBrightness(uint8_t percentage)
    {
        ledDriver.setBrightness(percentage);
    }

    void setSolidSegments(const LedSegmentArray &segments)
    {
        solidColorLedSegments = segments;
    }

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    AddressableLedDriver ledDriver;
    LedSegmentArray targetLedSegments{};
    LedSegmentArray solidColorLedSegments{};

    RainbowAnimation rainbowAnimation{targetLedSegments};
};