#pragma once

#include "core/SafeAssert.h"
#include "helpers/freertos.hpp"
#include "timers.h"
#include "wrappers/Task.hpp"

#include "tube-control/TubeControl.hpp"

class Clock : public util::wrappers::TaskWithMemberFunctionBase
{

public:
    Clock(TimerHandle_t &timeoutTimerHandle, TubeControl &tubeControl)   //
        : TaskWithMemberFunctionBase("clockTask", 128, osPriorityHigh2), //
          timeoutTimerHandle(timeoutTimerHandle),                        //
          tubeControl(tubeControl)                                       //
    {
        SafeAssert(timeoutTimerHandle != nullptr);
    }

    void timeSyncInterrupt();
    void timeoutInterrupt();

    void setClock(AbstractTube::Clock_t clockTime)
    {
        this->clockTime = clockTime;
    }

protected:
    void taskMain(void *);

private:
    TimerHandle_t &timeoutTimerHandle;
    TubeControl &tubeControl;

    static constexpr auto NotifyTimeSync = 1;
    static constexpr auto NotifyTimeout = 2;

    bool isInFallback = false;

    void incrementSecond();

    AbstractTube::Clock_t clockTime;
};