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

    // multiplexing will do by interrupts , this task is only for state machine purposes
    while (true)
    {
        switch (state)
        {
        case State::Standby:
            HAL_TIM_OC_Stop(multiplexingPwmTimer, fadingTimChannel);
            tubes->shutdownAllTubesAndDots();
            tubes->setBoostConverterState(false);
            break;

        case State::Clock:
            HAL_TIM_OC_Start(multiplexingPwmTimer, fadingTimChannel);
            tubes->setBoostConverterState(true);
            break;

        case State::Text:
            HAL_TIM_OC_Start(multiplexingPwmTimer, fadingTimChannel);
            tubes->setBoostConverterState(true);
            break;
        }
        notifyWait(0, UINT32_MAX, nullptr, portMAX_DELAY);
    }
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
void TubeControl::setClock(Time clockTime)
{
    tubes->prevClockTime = tubes->currentClockTime;
    tubes->currentClockTime = clockTime;
    tubes->enableDots(clockTime.second % 2 == 0);

    isFading = true;
    multiplexingCounter = 0;

    __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, Dimming::PwmMaximum);
    __HAL_TIM_ENABLE_IT(multiplexingPwmTimer, TIM_IT_CC2); // fadingTimChannel
}

//--------------------------------------------------------------------------------------------------
void TubeControl::multiplexingTimerInterrupt()
{
    if (state == State::Standby)
        return;

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