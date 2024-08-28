#include "Clock.hpp"
#include "esp_gateway/protocol.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

#include <climits>

void Clock::taskMain(void *)
{
    // enable time sync interrupt
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    while (true)
    {
        // wait for time sync pulse arriving every second
        uint32_t notificationValue = 0;
        notifyWait(ULONG_MAX, ULONG_MAX, &notificationValue, portMAX_DELAY);

        if ((syncEventGroup.getBits() & sync_events::TimeSyncArrived) == 0)
        {
            // ESP is sending time sync pulses and we doesnt knowing the time yet
            // so we need to request the time from the ESP

            requestTimeFromEsp();
            continue;
        }

        // start/reset fallback timer
        xTimerReset(timeoutTimerHandle, portMAX_DELAY);

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
                tubeControl.setClock(clockTime);
            }
        }
        break;

        case NotifyTimeout:
        {
            if (isInFallback)
            {
                incrementSecond();
                tubeControl.setClock(clockTime);
            }

            else
            {
                // no time sync from ESP, go in fallback and internal second switches
                isInFallback = true;
                xTimerChangePeriod(timeoutTimerHandle, toOsTicks(1.0_s), portMAX_DELAY);

                // increment twice to compensate offset
                incrementSecond();
                incrementSecond();
                tubeControl.setClock(clockTime);
            }
        }
        break;

        default:
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Clock::requestTimeFromEsp()
{
    std::string topic = "clock";
    std::string payload = "request";
    PacketHeader packetHeader = {.topicLength = (uint16_t)topic.length(),
                                 .payloadSize = (uint16_t)payload.length()};

    txStream.send(std::span(reinterpret_cast<uint8_t *>(&packetHeader), sizeof(PacketHeader)),
                  portMAX_DELAY);
    txStream.send(std::span(reinterpret_cast<uint8_t *>(topic.data()), topic.length()),
                  portMAX_DELAY);
    txStream.send(std::span(reinterpret_cast<uint8_t *>(payload.data()), payload.length()),
                  portMAX_DELAY);
}

//--------------------------------------------------------------------------------------------------
void Clock::incrementSecond()
{
    if (shouldResetSeconds)
    {
        shouldResetSeconds = false;

        if (clockTime.second >= 30)
            incrementMinute(); // round up

        clockTime.second = 0;
    }
    else
        clockTime.addSeconds(1);
}

//--------------------------------------------------------------------------------------------------
void Clock::incrementMinute()
{
    clockTime.addMinutes(1);
}

//--------------------------------------------------------------------------------------------------
void Clock::incrementHour()
{
    clockTime.addHours(1);
}

//--------------------------------------------------------------------------------------------------
void Clock::resetSecondsAtNextTimeSync()
{
    shouldResetSeconds = true;
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
