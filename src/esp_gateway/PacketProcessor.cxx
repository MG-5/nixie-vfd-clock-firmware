#include "PacketProcessor.hpp"
#include "LED/LedDataTypes.hpp"
#include "protocol.hpp"
#include "sync.hpp"

#include <algorithm>
#include <cstring>
#include <string>

void PacketProcessor::taskMain(void *)
{
    while (true)
    {
        if (!extractPacketFromReceiveBuffer())
            continue;

        processPacket();
    }
}

/* possible topics:
    - state
    - brightness
    - led/state
    - led/segments
    - clock
    - reset
*/
//-----------------------------------------------------------------------------
void PacketProcessor::processPacket()
{
    std::string topicString{reinterpret_cast<char *>(topic), header.topicLength};
    std::string payloadString = {reinterpret_cast<char *>(payload), header.payloadSize};

    if (topicString == "state")
    {
        // make the string lowercase
        std::transform(payloadString.begin(), payloadString.end(), payloadString.begin(),
                       ::tolower);

        if (payloadString == "standby")
            tubeControl.state = TubeControl::State::Standby;

        else if (payloadString == "clock")
            tubeControl.state = TubeControl::State::Clock;

        else if (payloadString == "text")
            // ToDo
            // tubeControl.state = TubeControl::State::Text;
            asm("nop");

        tubeControl.notify(1, util::wrappers::NotifyAction::SetBits);
    }
    else if (topicString == "brightness")
    {
        // only numbers are allowed in payload
        if (std::all_of(payloadString.begin(), payloadString.end(), ::isdigit))
        {
            // convert string to integer
            uint8_t newBrightness = std::clamp(std::atoi(payloadString.c_str()), 1, 100);
            tubeControl.dimming.setBrightness(newBrightness);
        }
    }
    else if (topicString == "led/state")
    {
        // make the string lowercase
        std::transform(payloadString.begin(), payloadString.end(), payloadString.begin(),
                       ::tolower);

        if (payloadString == "off")
            lightController.currentAnimation = LightController::AnimationType::Off;

        else if (payloadString == "rainbow")
            lightController.currentAnimation = LightController::AnimationType::Rainbow;

        else if (payloadString == "solid")
            lightController.currentAnimation = LightController::AnimationType::SolidColor;
    }
    else if (topicString == "led/segments")
    {
        asm("nop");
        // ToDo
    }
    else if (topicString == "clock")
    {
        if (header.payloadSize != sizeof("HH:MM:SS") - 1)
            return;

        std::string clockPayload{reinterpret_cast<char *>(payload), header.payloadSize};
        Time newClock{clockPayload};
        clock.setClock(newClock);
        syncEventGroup.setBits(sync_events::TimeSyncArrived);
    }
    else if (topicString == "reset")
    {
        NVIC_SystemReset();
    }
}

//-----------------------------------------------------------------------------
bool PacketProcessor::extractPacketFromReceiveBuffer()
{
    topic = nullptr;
    payload = nullptr;

    if (bufferStartPosition == bufferLastPosition)
    {
        bufferStartPosition = 0;
        bufferLastPosition = 0;
    }

    const auto NumberOfBytes = rxStream.receive(
        std::span(rxBuffer + bufferLastPosition, RxBufferSize - bufferLastPosition), portMAX_DELAY);

    if (NumberOfBytes == 0)
        return false; // no bytes received

    bufferLastPosition += NumberOfBytes;

    if (bufferLastPosition >= RxBufferSize && bufferStartPosition > 0)
    {
        // shift buffer to the beginning to make space for new data
        std::memmove(rxBuffer, rxBuffer + bufferStartPosition,
                     bufferLastPosition - bufferStartPosition);

        bufferStartPosition = 0;
        bufferLastPosition -= bufferStartPosition;
    }

    while (true)
    {
        if (bufferLastPosition - bufferStartPosition < sizeof(PacketHeader))
        {
            // no header to read in available in buffer
            // wait for new bytes from UART
            return false;
        }

        std::memcpy(&header, rxBuffer + bufferStartPosition, sizeof(PacketHeader));

        if (header.magic == ProtocolMagic)
            break; // header found

        // no valid packet header found in buffer
        // get rid the first byte in buffer and try it again
        bufferStartPosition++;
    }

    if (header.topicLength + header.payloadSize >
        (bufferLastPosition - bufferStartPosition) - sizeof(PacketHeader))
    {
        // not enough data for the expected topic and payload
        // wait for new bytes from UART
        return false;
    }

    topic = rxBuffer + bufferStartPosition + sizeof(PacketHeader);

    if (header.payloadSize > 0)
        payload = topic + header.topicLength;

    bufferStartPosition += sizeof(PacketHeader) + header.topicLength + header.payloadSize;
    return true;
}