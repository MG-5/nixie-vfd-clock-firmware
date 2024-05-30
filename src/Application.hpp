#pragma once

#include "spi.h"
#include "tim.h"

#include "LED/LightController.hpp"
#include "tube-control/TubeControl.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    static constexpr auto DelayTimer = &htim2;
    static constexpr auto MultiplexingPwmTimer = &htim1;
    static constexpr auto LedSpiPeripherie = &hspi2;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

    static void multiplexingTimerUpdate();
    static void pwmTimerCompare();

private:
    static inline Application *instance{nullptr};

    void registerCallbacks();

    TubeControl tubeControl{MultiplexingPwmTimer, DelayTimer};
    LightController lightController{LedSpiPeripherie};
};
