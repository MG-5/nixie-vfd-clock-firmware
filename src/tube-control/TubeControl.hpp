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
    TubeControl(TIM_HandleTypeDef *multiplexingPwmTimer, TIM_HandleTypeDef *delayTimer,
                uint32_t pwmTimChannel, uint32_t fadingTimChannel)
        : TaskWithMemberFunctionBase("tubeControlTask", 256, osPriorityRealtime5), //
          dimming(multiplexingPwmTimer, pwmTimChannel),                            //
          multiplexingPwmTimer(multiplexingPwmTimer),                              //
          delayTimer(delayTimer),                                                  //
          fadingTimChannel(fadingTimChannel)                                       //
    {
        initClockType();
    }

    void initClockType();

    void multiplexingTimerInterrupt();
    void pwmTimerInterrupt();
    void fadingTimerInterrupt();

    void setClock(AbstractTube::Clock_t clockTime);

    Dimming dimming;

protected:
    void taskMain(void *) override;

private:
    TIM_HandleTypeDef *multiplexingPwmTimer;
    TIM_HandleTypeDef *delayTimer;
    uint32_t fadingTimChannel;

    AbstractTube *tubes = nullptr;
    util::Gpio selectGpio{NixieVfdSelect_GPIO_Port, NixieVfdSelect_Pin};

    void initPwm();

    bool isFading = false;
    uint16_t multiplexingCounter = 0;
    bool allowInterruptCall = false;
};
