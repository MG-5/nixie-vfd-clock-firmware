#pragma once

#include "spi.h"
#include "tim.h"
#include "usart.h"

#include "LED/LightController.hpp"
#include "clock/Clock.hpp"
#include "esp_gateway/PacketProcessor.hpp"
#include "helpers/freertos.hpp"
#include "tube-control/TubeControl.hpp"

#include "wrappers/StreamBuffer.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    static constexpr auto UartPeripherie = &huart1;
    static constexpr auto MultiplexingPwmTimer = &htim1;
    static constexpr auto LedSpiPeripherie = &hspi2;
    static constexpr auto PwmTimChannel = TIM_CHANNEL_1;
    static constexpr auto FadingTimChannel = TIM_CHANNEL_2;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

    static void multiplexingTimerUpdate();
    static void pwmTimerCompare();
    static void fadingTimerCompare();

    static void clockSecondTimeout(TimerHandle_t);

    TubeControl tubeControl{MultiplexingPwmTimer, PwmTimChannel, FadingTimChannel};
    LightController lightController{LedSpiPeripherie};

    TimerHandle_t timerTimeoutHandle =
        xTimerCreate("timeoutTimer", toOsTicks(2.0_s), pdFALSE, (void *)0, clockSecondTimeout);

    static constexpr auto TxBufferSize = 64;
    util::wrappers::StreamBuffer txStream{TxBufferSize, 0};

    Clock clock{timerTimeoutHandle, tubeControl, txStream};

    // gateway to ESP32
    PacketProcessor packetProcessor{UartPeripherie, clock, tubeControl, lightController};
    UartTx uartTx{UartPeripherie, txStream};

private:
    static inline Application *instance{nullptr};

    void registerCallbacks();
};
