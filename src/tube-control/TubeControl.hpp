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
          pwmTimChannel(pwmTimChannel){};

    void multiplexingTimerInterrupt();
    void pwmTimerInterrupt();

protected:
    void taskMain(void *) override;

private:
    TIM_HandleTypeDef *multiplexingPwmTimer;
    TIM_HandleTypeDef *delayTimer;
    uint32_t pwmTimChannel;

    // Nixie nixieTubes{};
    VFD vfdTubes{delayTimer};
    AbstractTube *tubes = &vfdTubes; //&nixieTubes;

    static constexpr auto MultiplexingTimeout = 5.0_ms;
};
