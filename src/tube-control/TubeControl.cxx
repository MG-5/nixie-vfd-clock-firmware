#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

#include "core/SafeAssert.h"

void TubeControl::taskMain(void *)
{
    // wait for steady voltages
    vTaskDelay(toOsTicks(100.0_ms));
    tubes->setup();
    vTaskDelay(toOsTicks(100.0_ms));

    dimming.initPwm();
    dimming.setBrightness(40);

    HAL_TIM_OC_Start_IT(multiplexingPwmTimer, fadingTimChannel);

    // further works will doing by interrupts
    vTaskSuspend(nullptr);
}

//--------------------------------------------------------------------------------------------------
void TubeControl::initClockType()
{
    bool isNixieClock = selectGpio.read();

    if (isNixieClock)
        tubes = new Nixie();

    else
        tubes = new VFD();
};

//--------------------------------------------------------------------------------------------------
void TubeControl::setClock(AbstractTube::Clock_t clockTime)
{
    tubes->prevClockTime = tubes->currentClockTime;
    tubes->currentClockTime = clockTime;
    tubes->enableDots(clockTime.seconds % 2 == 0);

    isFading = true;
    multiplexingCounter = 0;

    __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, Dimming::PwmMaximum + 1);
    HAL_TIM_OC_Start_IT(multiplexingPwmTimer, fadingTimChannel);
}

//--------------------------------------------------------------------------------------------------
void TubeControl::multiplexingTimerInterrupt()
{
    tubes->multiplexingStep(isFading);

    const auto StepsPerFadingPeriod = tubes->getStepsPerFadingPeriod();

    if (multiplexingCounter < StepsPerFadingPeriod)
    {
        const size_t Diff = StepsPerFadingPeriod - multiplexingCounter;
        const auto FadingValue =
            util::mapValue<size_t, size_t>(0, StepsPerFadingPeriod, Dimming::PwmMinimum, 169, Diff);

        allowInterruptCall = true;
        __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, FadingValue);
    }
    else if (multiplexingCounter == StepsPerFadingPeriod)
    {
        isFading = false;
        HAL_TIM_OC_Stop_IT(multiplexingPwmTimer, fadingTimChannel);
    }

    multiplexingCounter++;
}

//--------------------------------------------------------------------------------------------------
void TubeControl::pwmTimerInterrupt()
{
    tubes->disableAllTubes();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::fadingTimerInterrupt()
{
    if (allowInterruptCall)
    {
        allowInterruptCall = false;
        tubes->updateFadingDigit();
    }
}