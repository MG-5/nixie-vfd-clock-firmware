#pragma once

#include "main.h"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "AbstractTube.hpp"
#include "nixie/Nixie.hpp"
#include "vfd/VFD.hpp"

class TubeControl : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    TubeControl(TIM_HandleTypeDef *multiplexingPwmTimer, TIM_HandleTypeDef *delayTimer,
                uint32_t pwmTimChannel)
        : TaskWithMemberFunctionBase("tubeControlTask", 256, osPriorityRealtime5), //
          multiplexingPwmTimer(multiplexingPwmTimer),                              //
          delayTimer(delayTimer),                                                  //
          pwmTimChannel(pwmTimChannel)
    {
        bool isNixieClock = selectGpio.read();

        if (isNixieClock)
            tubes = new Nixie();

        else
            tubes = new VFD(delayTimer);
    };

    void multiplexingTimerInterrupt();
    void pwmTimerInterrupt();

    void setClock(AbstractTube::Clock_t clockTime);

protected:
    void taskMain(void *) override;

private:
    TIM_HandleTypeDef *multiplexingPwmTimer;
    TIM_HandleTypeDef *delayTimer;
    uint32_t pwmTimChannel;

    AbstractTube *tubes = nullptr;
    util::Gpio selectGpio{NixieVfdSelect_GPIO_Port, NixieVfdSelect_Pin};

    static constexpr auto MultiplexingTimeout = 5.0_ms;
};
