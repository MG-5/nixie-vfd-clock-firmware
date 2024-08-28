#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

#include "core/SafeAssert.h"

void TubeControl::taskMain(void *)
{
    // wait for steady voltages
    vTaskDelay(toOsTicks(100.0_ms));
    tubes->setup();
    vTaskDelay(toOsTicks(100.0_ms));
    // tubes->renderInitialization();
    // vTaskDelay(toOsTicks(1.0_s));

    // start multiplexing
    dimming.startTimerWithInterrupts();
    dimming.setBrightness(80);

    // multiplexing will done by interrupts , this task is only for state machine purposes
    while (true)
    {
        notifyWait(0, UINT32_MAX, nullptr, portMAX_DELAY);

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
            displayClock();
            break;

        case State::Text:
            HAL_TIM_OC_Start(multiplexingPwmTimer, fadingTimChannel);
            tubes->setBoostConverterState(true);
            break;
        }
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
    currentClockTime = clockTime;

    if (state != State::Clock)
        return;

    displayClock();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::setText(std::string &newText)
{
    text = newText;

    if (state != State::Text)
        return;

    displayText();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::displayClock()
{
    tubes->renderClock(currentClockTime);
    resetFading();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::resetFading()
{
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

    // calculate compare register value needed for fading and set it
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

    // do the actual multiplexing
    tubes->multiplexingStep(isFading);

    // prepare digit for fading to it by e.g. writing its data to shift register without latching
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