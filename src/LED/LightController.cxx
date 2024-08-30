#include "FreeRTOS.h"
#include "helpers/freertos.hpp"
#include "task.h"

#include "LightController.hpp"

#include <algorithm>
#include <climits>

using util::wrappers::NotifyAction;

[[noreturn]] void LightController::taskMain(void *)
{
    solidColorLedSegments.fill({0, 0, 255});
    while (true)
    {
        units::si::Time targetAnimationDelay = 50.0_ms;

        switch (currentAnimation)
        {
        case AnimationType::Off:
            ledDriver.sendZeroBuffer();
            break;

        case AnimationType::Rainbow:
            rainbowAnimation.doAnimationStep();
            targetAnimationDelay = rainbowAnimation.getDelay();
            ledDriver.sendBuffer(targetLedSegments);
            break;

        case AnimationType::SolidColor:
            ledDriver.sendBuffer(solidColorLedSegments);
            break;
        }

        vTaskDelay(toOsTicks(targetAnimationDelay));
    }
}

//--------------------------------------------------------------------------------------------------
void LightController::notifySpiIsFinished()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}