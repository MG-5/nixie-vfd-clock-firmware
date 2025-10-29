#pragma once

#include "main.h"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "BaseTubeDisplay.hpp"
#include "Dimming.hpp"
#include "nixie/Nixie.hpp"
#include "vfd/VFD.hpp"

class TubeControl : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    TubeControl(TIM_HandleTypeDef *multiplexingPwmTimer, uint32_t pwmTimChannel,
                uint32_t fadingTimChannel)
        : TaskWithMemberFunctionBase("tubeControlTask", 256, osPriorityRealtime5), //
          dimming(multiplexingPwmTimer, pwmTimChannel),                            //
          multiplexingPwmTimer(multiplexingPwmTimer),                              //
          fadingTimChannel(fadingTimChannel)                                       //
    {
        initClockType();
    }

    void initClockType();

    void multiplexingTimerInterrupt();
    void pwmTimerInterrupt();
    void fadingTimerInterrupt();

    void updateClock(Time clockTime);
    void updateText(std::string &newText);

    void showSeconds(bool newValue)
    {
        shouldShowSeconds = newValue;
    }

    void setTubeBrightness(uint8_t newBrightness)
    {
        brightness = newBrightness;
        dimming.setBrightness(brightness);
    }

    enum class State
    {
        Standby,
        Clock,
        Text
    };

    State currentState = State::Clock;
    State prevState = State::Clock;

    void updateState(State newState)
    {
        if (currentState != State::Standby)
            prevState = currentState; // save last active state

        currentState = newState;
        notify(1, util::wrappers::NotifyAction::SetBits);
    }

    void enableDisplayBlinkingSixTimes();
    void disableDisplayBlinking();

    bool isBlinking() const
    {
        return shouldDisplayBlinking;
    }

protected:
    void taskMain(void *) override;

private:
    Dimming dimming;
    TIM_HandleTypeDef *multiplexingPwmTimer;
    uint32_t fadingTimChannel;

    BaseTubeDisplay *tubeDisplay = nullptr;
    util::Gpio selectGpio{NixieVfdSelect_GPIO_Port, NixieVfdSelect_Pin};

    uint8_t brightness = 80;
    bool isFading = false;
    uint16_t multiplexingCounter = 0;
    bool allowInterruptCall = false;
    bool shouldShowSeconds = true;

    bool shouldDisplayBlinking = false;
    uint8_t blinkingCounter = 0;
    uint16_t stepCounter = 0;

    Time currentClockTime;
    std::string text = "";

    void displayClock();
    void displayText();
    void resetFading();
};
