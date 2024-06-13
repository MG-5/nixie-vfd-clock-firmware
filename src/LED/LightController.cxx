#include "FreeRTOS.h"
#include "helpers/freertos.hpp"
#include "task.h"

#include "LightController.hpp"

#include <algorithm>
#include <climits>

using util::wrappers::NotifyAction;

[[noreturn]] void LightController::taskMain(void *)
{
    while (true)
    {
        updateLightState();

        targetAnimation->doAnimationStep();
        ledDriver.sendBuffer(ledSegmentArray);

        vTaskDelay(toOsTicks(targetAnimation->getDelay()));
    }
}

//--------------------------------------------------------------------------------------------------
void LightController::notifySpiIsFinished()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void LightController::updateLightState()
{
}