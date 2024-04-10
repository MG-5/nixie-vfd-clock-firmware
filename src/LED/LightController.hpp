#pragma once

#include "AddressableLedDriver.hpp"

#include "animations/LedAnimationBase.hpp"
#include "animations/RainbowAnimation.hpp"

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

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    AddressableLedDriver ledDriver;
    LedSegmentArray ledSegmentArray{};

    RainbowAnimation rainbowAnimation{ledSegmentArray};
    LedAnimationBase *targetAnimation{&rainbowAnimation};

    void updateLightState();
};