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
        units::si::Time targetAnimationDelay = 20.0_ms;

        switch (currentAnimation)
        {
        case AnimationType::Off:
            targetLedSegments.fill({0, 0, 0});
            break;

        case AnimationType::Rainbow:
            rainbowAnimation.doAnimationStep();
            targetAnimationDelay = rainbowAnimation.getDelay();

            break;

        case AnimationType::SolidColor:
            targetLedSegments = solidColorLedSegments;
            break;
        }

        ledDriver.sendBuffer(targetLedSegments);
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