#pragma once

// #include "spi.h"

// #include "LED/LightController.hpp"
#include "tube-control/TubeControl.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

    // static constexpr auto LedSpiPeripherie = &hspi2;
    // TODO Add SpiAccessor

    // LightController lightController{LedSpiPeripherie};
    TubeControl tubeControl{};

private:
    static inline Application *instance{nullptr};

    HAL_StatusTypeDef registerCallbacks();
};
