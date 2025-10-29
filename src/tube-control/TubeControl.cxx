#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

#include "core/SafeAssert.h"

void TubeControl::taskMain(void *)
{
    // wait for steady steady input
    vTaskDelay(toOsTicks(100.0_ms));
    tubeDisplay->powerOn();

    // wait for tubes to warm up
    vTaskDelay(toOsTicks(100.0_ms));

    // show gimmick initialization
    // tubeDisplay->renderInitialization();
    // vTaskDelay(toOsTicks(1.0_s));

    // start multiplexing
    dimming.startTimerWithInterrupts();
    dimming.setBrightness(brightness);

    // multiplexing will done by interrupts , this task is only for state machine purposes
    while (true)
    {
        // wait for state machine changes
        notifyWait(0, UINT32_MAX, nullptr, portMAX_DELAY);

        if (currentState == State::Standby)
        {
            HAL_TIM_OC_Stop(multiplexingPwmTimer, fadingTimChannel);
            tubeDisplay->shutdownAllTubesAndDots();
            tubeDisplay->powerOff();
        }
        else
        {
            HAL_TIM_OC_Start(multiplexingPwmTimer, fadingTimChannel);
            tubeDisplay->powerOn();
        }

        switch (currentState)
        {
        case State::Clock:
            displayClock();
            break;

        case State::Text:
            displayText();
            break;

        case State::Standby:
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void TubeControl::initClockType()
{
    bool isNixieClock = selectGpio.read();

    if (isNixieClock)
        tubeDisplay = new Nixie();

    else
        tubeDisplay = new VFD();
};

//--------------------------------------------------------------------------------------------------
void TubeControl::updateClock(Time clockTime)
{
    currentClockTime = clockTime;

    if (currentState != State::Clock)
        return;

    displayClock();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::updateText(std::string &newText)
{
    text = newText;

    if (currentState != State::Text)
        return;

    displayText();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::displayClock()
{
    tubeDisplay->renderClock(currentClockTime);
    resetFading();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::displayText()
{
    tubeDisplay->renderText(text);
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
    if (currentState == State::Standby)
        return;

    if (shouldDisplayBlinking)
    {
        // only increase counter every 500ms
        stepCounter++;
        if (stepCounter >=
            (500.0_ms / BaseTubeDisplay::MultiplexingStepPeriod).getMagnitude<size_t>())
        {
            blinkingCounter++;
            stepCounter = 0;
        }

        if (blinkingCounter >= 6 * 2) // times on/off
        {
            shouldDisplayBlinking = false;
            blinkingCounter = 0;
        }
        else
        {
            // blink off on odd counts
            if ((blinkingCounter % 2) == 1)
            {
                tubeDisplay->shutdownCurrentTubeAndDot();
                return;
            }
        }
    }

    // calculate compare register value needed for fading and set it
    if (multiplexingCounter < BaseTubeDisplay::StepsPerFadingPeriod && brightness >= 25)
    {
        const size_t Diff = BaseTubeDisplay::StepsPerFadingPeriod - multiplexingCounter;
        const auto FadingValue =
            util::mapValue<size_t, size_t>(0, BaseTubeDisplay::StepsPerFadingPeriod,
                                           Dimming::PwmMinimum, Dimming::PwmMaximum, Diff);

        allowInterruptCall = true;
        __HAL_TIM_SET_COMPARE(multiplexingPwmTimer, fadingTimChannel, FadingValue);
    }
    else
    {
        isFading = false;
        __HAL_TIM_DISABLE_IT(multiplexingPwmTimer, TIM_IT_CC2); // fadingTimChannel
    }

    // do the actual multiplexing
    tubeDisplay->multiplexingStep(isFading, shouldShowSeconds);

    // prepare digit for fading to it by e.g. writing its data to shift register without
    // latching
    if (isFading)
        tubeDisplay->prepareFadingDigit();

    multiplexingCounter++;
}

//--------------------------------------------------------------------------------------------------
void TubeControl::pwmTimerInterrupt()
{
    tubeDisplay->shutdownCurrentTubeAndDot();
}

//--------------------------------------------------------------------------------------------------
void TubeControl::fadingTimerInterrupt()
{
    if (allowInterruptCall)
    {
        allowInterruptCall = false;
        tubeDisplay->updateFadingDigit();
    }
}

//--------------------------------------------------------------------------------------------------
void TubeControl::enableDisplayBlinkingSixTimes()
{
    shouldDisplayBlinking = true;
    blinkingCounter = 0;
}

//--------------------------------------------------------------------------------------------------
void TubeControl::disableDisplayBlinking()
{
    shouldDisplayBlinking = false;
}