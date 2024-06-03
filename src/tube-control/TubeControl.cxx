#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

#include "core/SafeAssert.h"

void TubeControl::taskMain(void *)
{
    // wait for steady voltages
    vTaskDelay(toOsTicks(100.0_ms));
    tubes->setup();
    vTaskDelay(toOsTicks(100.0_ms));

    // tubes->setClock(18, 42, 05);

    uint16_t counter = 0;
    uint8_t number = 0;
    bool enableDots = false;

    HAL_TIM_Base_Start_IT(multiplexingPwmTimer);
    HAL_TIM_OC_Start_IT(multiplexingPwmTimer, pwmTimChannel);

    constexpr auto PwmMinimum = 50;
    constexpr auto PwmMaximum = 249;
    __HAL_TIM_SetCompare(multiplexingPwmTimer, pwmTimChannel, PwmMaximum);

    while (1)
    {
        notifyTake(toOsTicks(MultiplexingTimeout));

        tubes->multiplexingStep();
    }
}

//--------------------------------------------------------------------------------------------------
void TubeControl::setClock(AbstractTube::Clock_t clockTime)
{
    tubes->setClock(clockTime);
    tubes->enableDots(clockTime.seconds % 2 == 0);
}

//--------------------------------------------------------------------------------------------------
void TubeControl::multiplexingTimerInterrupt()
{
    // APB2 = 64MHz -> 1MHz = 1µs -> prescaler 64-1
    // auto reload period = 249 -> interrupt every 250µs

    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, util::wrappers::NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void TubeControl::pwmTimerInterrupt()
{
    tubes->disableAllTubes();
}
