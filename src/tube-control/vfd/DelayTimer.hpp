#pragma once

#include "tim.h"

// APB1= 32MHz -> prescaler = 2-1 -> 16MHz -> 62,5ns
class DelayTimer
{
public:
    DelayTimer(TIM_HandleTypeDef *timer)
        : timer(timer){

          };

    void startTimer()
    {
        HAL_TIM_Base_Start(timer);
    }

    bool isTimerRunning()
    {
        return (timer->Instance->CR1 & TIM_CR1_CEN);
    }

    void delay187ns()
    {
        if (!isTimerRunning())
            return;

        __HAL_TIM_SetCounter(timer, 0);

        while (__HAL_TIM_GET_COUNTER(timer) < 3)
        {
        };
    }

    void delay250ns()
    {
        if (!isTimerRunning())
            return;

        __HAL_TIM_SetCounter(timer, 0);

        while (__HAL_TIM_GET_COUNTER(timer) < 4)
        {
        };
    }

private:
    TIM_HandleTypeDef *timer;
};