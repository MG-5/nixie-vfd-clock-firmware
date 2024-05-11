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

    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    // __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 120);

    size_t pwmCounter = 0;
    size_t pwmThreshold = 0;
    constexpr auto PwmMaximum = 5;

    while (1)
    {
        notifyTake(toOsTicks(MultiplexingTimeout));

        if (++pwmCounter >= pwmThreshold)
            tubes->multiplexingStep();
        else
            vfdTubes.disableAll();

        if (pwmCounter >= PwmMaximum)
            pwmCounter = 0;

        constexpr auto NumberStepsForOneSecond = (1.0_s / 125.0_us).getMagnitude<size_t>();
        if (++counter >= NumberStepsForOneSecond)
        {
            if (++pwmThreshold > PwmMaximum)
                pwmThreshold = 0;

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
    // APB1 = 32MHz -> 1MHz = 1µs -> prescaler 32-1
    // auto reload period = 124 -> interrupt every 125µs

    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, util::wrappers::NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
