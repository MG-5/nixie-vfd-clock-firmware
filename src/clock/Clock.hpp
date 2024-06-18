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

    void resetSecondsAtNextTimeSync();

    void setClock(AbstractTube::Clock_t clockTime)
    {
        this->clockTime = clockTime;
    }

    void setSecond(uint8_t second)
    {
        if (second < 60)
            clockTime.seconds = second;
    }

    void setMinute(uint8_t minute)
    {
        if (minute < 60)
            clockTime.minutes = minute;
    }

    void setHour(uint8_t hour)
    {
        if (hour < 24)
            clockTime.hours = hour;
    }

protected:
    void taskMain(void *);

private:
    TimerHandle_t &timeoutTimerHandle;
    TubeControl &tubeControl;

    static constexpr auto NotifyTimeSync = 1;
    static constexpr auto NotifyTimeout = 2;

    bool isInFallback = false;
    bool shouldResetSeconds = false;

    void incrementSecond();
    void incrementMinute();
    void incrementHour();

    AbstractTube::Clock_t clockTime;
};