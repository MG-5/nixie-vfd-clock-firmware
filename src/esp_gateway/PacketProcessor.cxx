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
    - text
    - reset
*/
//-----------------------------------------------------------------------------
void PacketProcessor::processPacket()
{
    // ToDO: send back current state changes

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
            tubeControl.state = TubeControl::State::Text;

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
    else if (topicString == "text")
    {
        std::string textPayload{reinterpret_cast<char *>(payload), header.payloadSize};
        // replace äöü with ae oe ue
        replaceUmlauts(textPayload);

        if (textPayload.size() < 6)
        {
            // add spaces to the end of the string to make it 6 characters long to fit on six tubes
            textPayload.append(6 - textPayload.size(), ' ');
        }
        tubeControl.setText(textPayload);

        if (tubeControl.state != TubeControl::State::Text)
        {
            tubeControl.state = TubeControl::State::Text;
            tubeControl.notify(1, util::wrappers::NotifyAction::SetBits);
        }
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

//-----------------------------------------------------------------------------
void PacketProcessor::replaceUmlauts(std::string &text)
{
    std::array<std::pair<std::string, std::string>, 7> umlauts = {
        std::make_pair("ä", "ae"), std::make_pair("ö", "oe"), std::make_pair("ü", "ue"),
        std::make_pair("ß", "ss"), std::make_pair("Ä", "AE"), std::make_pair("Ö", "OE"),
        std::make_pair("Ü", "UE")};

    for (const auto &[umlaut, replacement] : umlauts)
    {
        while (true)
        {
            auto pos = text.find(umlaut);
            if (pos != std::string::npos)
                text.replace(pos, 2, replacement);

            else
                break;
        }
    }
}