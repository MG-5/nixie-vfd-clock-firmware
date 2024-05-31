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

        constexpr auto NumberStepsForOneSecond = (1.0_s / 250.0_us).getMagnitude<size_t>();
        if (++counter >= NumberStepsForOneSecond)
        {

            counter = 0;
            if (++number >= 10)
                number = 0;

            const auto numberToShow = number * 10 + number;
            tubes->setClock(numberToShow, numberToShow, numberToShow);
            enableDots = !enableDots;
            tubes->enableDots(enableDots);
        }
    }
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
