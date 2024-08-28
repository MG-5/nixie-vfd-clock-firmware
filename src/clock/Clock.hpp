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
    }

    void timeSyncInterrupt();
    void timeoutInterrupt();

    void resetSecondsAtNextTimeSync();

    void setClock(Time clockTime)
    {
        this->clockTime = clockTime;
    }

protected:
    void taskMain(void *);

private:
    TimerHandle_t &timeoutTimerHandle;
    TubeControl &tubeControl;
    util::wrappers::StreamBuffer &txStream;

    static constexpr auto NotifyTimeSync = 1 << 1;
    static constexpr auto NotifyTimeout = 1 << 2;

    bool isInFallback = false;
    bool shouldResetSeconds = false;

    void incrementSecond();
    void incrementMinute();
    void incrementHour();

    void requestTimeFromEsp();

    Time clockTime;
};