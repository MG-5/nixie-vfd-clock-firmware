#pragma once

#include "main.h"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "AbstractTube.hpp"
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

    void setClock(Time clockTime);
    void setText(std::string &newText);

    Dimming dimming;

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

protected:
    void taskMain(void *) override;

private:
    TIM_HandleTypeDef *multiplexingPwmTimer;
    uint32_t fadingTimChannel;

    AbstractTube *tubes = nullptr;
    util::Gpio selectGpio{NixieVfdSelect_GPIO_Port, NixieVfdSelect_Pin};

    bool isFading = false;
    uint16_t multiplexingCounter = 0;
    bool allowInterruptCall = false;

    Time currentClockTime;
    std::string text = "";

    void displayClock();
    void displayText();
    void resetFading();
};
