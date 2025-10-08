#pragma once

#include "core/SafeAssert.h"
#include "helpers/freertos.hpp"
#include "timers.h"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "tube-control/TubeControl.hpp"

class Clock : public util::wrappers::TaskWithMemberFunctionBase
{

public:
    Clock(TimerHandle_t &timeoutTimerHandle, TubeControl &tubeControl,
          util::wrappers::StreamBuffer &txStream)                        //
        : TaskWithMemberFunctionBase("clockTask", 128, osPriorityHigh2), //
          timeoutTimerHandle(timeoutTimerHandle),                        //
          tubeControl(tubeControl),                                      //
          txStream(txStream)
    {
        SafeAssert(timeoutTimerHandle != nullptr);
        xTimerChangePeriod(timeoutTimerHandle, toOsTicks(1.0_s + TimeoutPeriod), portMAX_DELAY);
    }

    void timeSyncInterrupt();
    void timeoutInterrupt();

    void updateMainClock(Time clockTime)
    {
        this->mainClock = clockTime;

        auto higherPriorityTaskWoken = pdFALSE;
        notifyFromISR(NotifyBits::TimeUpdated, util::wrappers::NotifyAction::SetBits,
                      &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

    enum class State
    {
        Normal,
        Countdown,
        Countup
    } state = State::Normal;

    bool isCountdownRunning = false;
    bool isCountupRunning = false;

protected:
    void taskMain(void *);

private:
    TimerHandle_t &timeoutTimerHandle;
    TubeControl &tubeControl;
    util::wrappers::StreamBuffer &txStream;

    struct NotifyBits
    {
        static constexpr auto TimeSync = 1 << 1;
        static constexpr auto Timeout = 1 << 2;
        static constexpr auto TimeUpdated = 1 << 3;
    };

    static constexpr auto TimeoutPeriod = 0.1_s;

    bool isInFallback = true;
    Time mainClock;

    void clockTick();
    void requestTimeFromEsp();
};