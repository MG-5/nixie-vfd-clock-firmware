#include "Clock.hpp"
#include "esp_gateway/protocol.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

#include <climits>

void Clock::taskMain(void *)
{
    while (true)
    {
        // wait for:
        // - time sync from ESP
        // - timeout (internal second tick in fallback mode)
        // - time update from ESP
        uint32_t notificationValue = 0;
        notifyWait(ULONG_MAX, ULONG_MAX, &notificationValue, portMAX_DELAY);

        // start/reset fallback timer
        xTimerReset(timeoutTimerHandle, portMAX_DELAY);

        switch (notificationValue)
        {
        case NotifyBits::TimeSync:
        {
            if (isInFallback)
            {
                // ESP is sending time syncs (again), request time
                isInFallback = false;
                xTimerChangePeriod(timeoutTimerHandle, toOsTicks(1.0_s + TimeoutPeriod),
                                   portMAX_DELAY);
                requestTimeFromEsp();
                continue;
            }
        }
        break;

        case NotifyBits::Timeout:
        {
            if (!isInFallback)
            {
                // no time sync from ESP, go in fallback and internal second switches
                isInFallback = true;
                xTimerChangePeriod(timeoutTimerHandle, toOsTicks(1.0_s), portMAX_DELAY);
            }
        }
        break;

        case NotifyBits::TimeUpdated:
            // time updated from ESP, just update display
            tubeControl.updateClock(mainClock);
            continue;

        default:
            break;
        }

        clockTick();
        tubeControl.updateClock(mainClock);
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
void Clock::clockTick()
{
    mainClock.addSeconds(1);
}

//--------------------------------------------------------------------------------------------------
void Clock::timeSyncInterrupt()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(NotifyBits::TimeSync, util::wrappers::NotifyAction::SetBits,
                  &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

//--------------------------------------------------------------------------------------------------
void Clock::timeoutInterrupt()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(NotifyBits::Timeout, util::wrappers::NotifyAction::SetBits,
                  &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
