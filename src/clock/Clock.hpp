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

    void timeSyncCallback();
    void timeoutCallback();

    void updateMainClock(Time clockTime)
    {
        state = State::Normal;
        this->mainClock = clockTime;
        notify(NotifyBits::TimeUpdated, util::wrappers::NotifyAction::SetBits);
    }

    void switchToMainClock()
    {
        waitForBlinkingToFinish = false;
        tubeControl.disableDisplayBlinking();
        state = State::Normal;
        isCountdownRunning = false;
        isCountupRunning = false;

        updateClockDisplay();
    }

    void setCountdownClock(Time countdownTime)
    {
        if (countdownTime.getSeconds() == 0)
            return;

        this->countdownClock = countdownTime;
        switchToCountdownClock();
    }

    void switchToCountdownClock()
    {
        if (countdownClock.getSeconds() == 0)
            return;

        waitForBlinkingToFinish = false;
        tubeControl.disableDisplayBlinking();
        state = State::Countdown;
        isCountdownRunning = true;

        updateClockDisplay();
    }

    void switchToCountupClock()
    {
        waitForBlinkingToFinish = false;
        tubeControl.disableDisplayBlinking();
        state = State::Countup;
        isCountupRunning = true;

        updateClockDisplay();
    }

    void resetCountupClock()
    {
        countupClock = Time{};
        isCountupRunning = false;

        updateClockDisplay();
    }

    void updateClockDisplay()
    {
        switch (state)
        {
        default:
        case State::Normal:
            tubeControl.updateClock(mainClock);
            break;

        case State::Countdown:
            tubeControl.updateClock(countdownClock);
            break;

        case State::Countup:
            tubeControl.updateClock(countupClock);
            break;
        }
    }

    void resetTimeout()
    {
        xTimerReset(timeoutTimerHandle, portMAX_DELAY);
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
    bool waitForBlinkingToFinish = false;
    Time mainClock;
    Time countdownClock;
    Time countupClock;

    void clockTick();
    void requestTimeFromEsp();
    void checkCountdownFinished();
};