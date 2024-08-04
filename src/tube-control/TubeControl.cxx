#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

#include "core/SafeAssert.h"

void TubeControl::taskMain(void *)
{
    // wait for steady voltages
    vTaskDelay(toOsTicks(100.0_ms));
    tubes->setup();
    vTaskDelay(toOsTicks(100.0_ms));

    dimming.initPwm(); // also starts multiplexing
    dimming.setBrightness(80);

    HAL_TIM_OC_Start(multiplexingPwmTimer, fadingTimChannel);

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

    __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, Dimming::PwmMaximum);
    __HAL_TIM_ENABLE_IT(multiplexingPwmTimer, TIM_IT_CC2); // fadingTimChannel
}

//--------------------------------------------------------------------------------------------------
void TubeControl::multiplexingTimerInterrupt()
{
    static const auto StepsPerFadingPeriod = tubes->getStepsPerFadingPeriod();
    if (multiplexingCounter < StepsPerFadingPeriod)
    {
        const size_t Diff = StepsPerFadingPeriod - multiplexingCounter;
        const auto FadingValue = util::mapValue<size_t, size_t>(
            0, StepsPerFadingPeriod, Dimming::PwmMinimum, Dimming::PwmMaximum, Diff);

        allowInterruptCall = true;
        __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, FadingValue);
    }
    else
    {
        isFading = false;
        __HAL_TIM_DISABLE_IT(multiplexingPwmTimer, TIM_IT_CC2); // fadingTimChannel
    }

    tubes->multiplexingStep(isFading);

    if (isFading)
        tubes->prepareFadingDigit();

    multiplexingCounter++;
}

//--------------------------------------------------------------------------------------------------
void TubeControl::pwmTimerInterrupt()
{
    tubes->shutdownCurrentTubeAndDot();
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