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
    - led/brightness
    - clock
    - text
    - reset
*/
//-----------------------------------------------------------------------------
void PacketProcessor::processPacket()
{
    // ToDo: send back current state changes

    std::string topicString{reinterpret_cast<char *>(topic), header.topicLength};
    std::string payloadString = {reinterpret_cast<char *>(payload), header.payloadSize};

    if (topicString == "state")
    {
        // make the string lowercase
        std::transform(payloadString.begin(), payloadString.end(), payloadString.begin(),
                       ::tolower);

        if (payloadString == "standby")
            tubeControl.updateState(TubeControl::State::Standby);

        else if (payloadString == "clock")
            tubeControl.updateState(TubeControl::State::Clock);

        else if (payloadString == "text")
            tubeControl.updateState(TubeControl::State::Text);
    }
    else if (topicString == "brightness")
    {
        // only numbers are allowed in payload
        if (std::all_of(payloadString.begin(), payloadString.end(), ::isdigit))
        {
            // convert string to integer
            uint8_t newBrightness = std::clamp(std::atoi(payloadString.c_str()), 0, 100);

            if (newBrightness == 0)
                tubeControl.updateState(TubeControl::State::Standby);

            else
            {
                tubeControl.dimming.setBrightness(newBrightness);

                // restore last active state
                if (tubeControl.currentState == TubeControl::State::Standby)
                    tubeControl.updateState(tubeControl.prevState);
            }
        }
    }
    else if (topicString == "led/state")
    {
        // make the string lowercase
        std::transform(payloadString.begin(), payloadString.end(), payloadString.begin(),
                       ::tolower);

        if (payloadString == "off")
            lightController.updateAnimationType(LightController::AnimationType::Off);

        else if (payloadString == "rainbow")
            lightController.updateAnimationType(LightController::AnimationType::Rainbow);

        else if (payloadString == "solid")
            lightController.updateAnimationType(LightController::AnimationType::SolidColor);
    }
    else if (topicString == "led/brightness")
    {
        // only numbers are allowed in payload
        if (std::all_of(payloadString.begin(), payloadString.end(), ::isdigit))
        {
            // convert string to integer
            uint8_t newBrightness = std::clamp(std::atoi(payloadString.c_str()), 0, 100);

            if (newBrightness == 0)
                lightController.updateAnimationType(LightController::AnimationType::Off);

            else
            {
                lightController.setBrightness(newBrightness);

                // restore last active animation
                if (lightController.currentAnimation == LightController::AnimationType::Off)
                    lightController.updateAnimationType(lightController.prevAnimation);
            }
        }
    }
    else if (topicString == "led/segments")
    {
        // format is [R,G,B][R,G,B]...
        std::string segmentPayload{reinterpret_cast<char *>(payload), header.payloadSize};

        // simple check for valid payload
        size_t countOpenBrackets = 0;
        size_t countCloseBrackets = 0;
        size_t countCommas = 0;
        for (const auto &c : segmentPayload)
        {
            if (c == '[')
                countOpenBrackets++;
            else if (c == ']')
                countCloseBrackets++;
            else if (c == ',')
                countCommas++;
        }

        if (countOpenBrackets != countCloseBrackets || //
            countCloseBrackets != NumberOfLeds ||      //
            countCommas != 2 * NumberOfLeds)
            return;

        LedSegmentArray segments;
        size_t ledIndex = 0;
        // first loop is searching for [
        // second loop is searching for , and ] to split the segments
        for (size_t i = 0, lastCommaIndex = 0; i < segmentPayload.size(); i++)
        {
            if (segmentPayload[i] == '[')
            {
                BgrColor segment;
                bool foundFirstComma = false;
                bool foundSecondComma = false;
                for (size_t j = i + 1; j < segmentPayload.size(); j++)
                {
                    if (segmentPayload[j] == ',')
                    {
                        if (!foundFirstComma)
                        { // value for red color
                            foundFirstComma = true;
                            segment.red = std::clamp(
                                std::atoi(segmentPayload.substr(i + 1, j - i - 1).c_str()), 0, 255);
                            lastCommaIndex = j;
                        }
                        else
                        { // value for green color
                            foundSecondComma = true;
                            segment.green = std::clamp(
                                std::atoi(segmentPayload
                                              .substr(lastCommaIndex + 1, j - lastCommaIndex - 1)
                                              .c_str()),
                                0, 255);
                            lastCommaIndex = j;
                        }
                    }
                    else if (segmentPayload[j] == ']')
                    {
                        if (!foundFirstComma || !foundSecondComma)
                            return; // ToDo: error handling

                        // value for blue color
                        segment.blue = std::clamp(
                            std::atoi(
                                segmentPayload.substr(lastCommaIndex + 1, j - lastCommaIndex - 1)
                                    .c_str()),
                            0, 255);
                        segments[ledIndex++] = segment;
                        break;
                    }
                    else if (segmentPayload[j] == '[')
                        return; // ToDo: error handling
                }
            }
        }
        if (ledIndex != NumberOfLeds)
            return;

        lightController.setSolidSegments(segments);
        lightController.updateAnimationType(LightController::AnimationType::SolidColor);
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
        std::string textPayload{
            reinterpret_cast<char *>(payload),
            std::min(header.payloadSize, (uint16_t)(AbstractTube ::NumberOfTubes * 2))};
        // replace äöü with ae oe ue
        replaceUmlauts(textPayload);

        if (textPayload.size() < 6)
        {
            // add spaces to the end of the string to make it 6 characters long to fit on 6 tubes
            textPayload.append(6 - textPayload.size(), ' ');
        }
        tubeControl.setText(textPayload);

        if (tubeControl.currentState != TubeControl::State::Text)
        {
            tubeControl.currentState = TubeControl::State::Text;
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