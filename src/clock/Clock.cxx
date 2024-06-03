#include "Clock.hpp"
#include "helpers/freertos.hpp"

#include <climits>

void Clock::taskMain(void *)
{
    // ToDo: wait for tube initialization

    // ToDo: wait for ESP clock setting by waiting for first time sync pulse but incrementing
    // seconds only starts with second pulse (or timeout)
    // notifyWait(ULONG_MAX, ULONG_MAX, (uint32_t *)0, portMAX_DELAY);

    uint32_t notificationValue = 0;

    while (true)
    {
        // start timeout timer
        xTimerReset(timeoutTimerHandle, portMAX_DELAY);

        // wait for time sync every second
        notificationValue = 0;
        notifyWait(ULONG_MAX, ULONG_MAX, &notificationValue, portMAX_DELAY);

        switch (notificationValue)
        {
        case NotifyTimeSync:
        {
            if (isInFallback)
            {
                // ESP is sending time syncs again, skip increment due fresh setting per UART
                isInFallback = false;
                xTimerChangePeriod(timeoutTimerHandle, toOsTicks(2.0_s), portMAX_DELAY);
            }
            else
            {
                xTimerReset(timeoutTimerHandle, portMAX_DELAY);
                incrementSecond();
            }
        }
        break;

        case NotifyTimeout:
        {
            if (isInFallback)
                incrementSecond();

            else
            {
                // no time sync from ESP, go in fallback and internal second switches
                isInFallback = true;
                xTimerChangePeriod(timeoutTimerHandle, toOsTicks(1.0_s), portMAX_DELAY);

                // increment twice to compensate offset
                incrementSecond();
                incrementSecond();
            }
        }
        break;

        default:
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Clock::incrementSecond()
{
    if (++clockTime.seconds >= 60)
    {
        clockTime.seconds = 0;

        if (++clockTime.minutes >= 60)
        {
            clockTime.minutes = 0;

            if (++clockTime.hours >= 24)
                clockTime.hours = 0;
        }
    }

    tubeControl.setClock(clockTime);
}

//--------------------------------------------------------------------------------------------------
void Clock::timeSyncInterrupt()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(NotifyTimeSync, util::wrappers::NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void Clock::timeoutInterrupt()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(NotifyTimeout, util::wrappers::NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
