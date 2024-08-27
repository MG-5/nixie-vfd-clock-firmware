#pragma once

#include "wrappers/EventGroup.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "LED/LightController.hpp"
#include "UartRx.hpp"
#include "UartTx.hpp"
#include "clock/Clock.hpp"
#include "protocol.hpp"
#include "tube-control/TubeControl.hpp"

class PacketProcessor : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    PacketProcessor(UART_HandleTypeDef *espUartPeripherie, Clock &clock, TubeControl &tubeControl,
                    LightController &lightController)
        : TaskWithMemberFunctionBase("packetProcessor", 128, osPriorityAboveNormal), //
          espUartPeripherie(espUartPeripherie),                                      //
          clock(clock),                                                              //
          tubeControl(tubeControl),                                                  //
          lightController(lightController)
    {
        SafeAssert(espUartPeripherie != nullptr);
    };

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    UART_HandleTypeDef *espUartPeripherie = nullptr;
    Clock &clock;
    TubeControl &tubeControl;
    LightController &lightController;

    static constexpr auto RxBufferSize = 256;
    uint8_t rxBuffer[RxBufferSize];
    util::wrappers::StreamBuffer rxStream{RxBufferSize, 1};

    static constexpr auto TxBufferSize = 64;
    uint8_t txBuffer[TxBufferSize];
    util::wrappers::StreamBuffer txStream{TxBufferSize, 0};

    size_t bufferStartPosition = 0;
    size_t bufferLastPosition = 0;

    uint8_t *topic = nullptr;
    uint8_t *payload = nullptr;
    PacketHeader header{};

    bool extractPacketFromReceiveBuffer();
    PacketHeader processPacket();
    void sendResponsePacket(PacketHeader &responseHeader);

public:
    UartTx uartTx{espUartPeripherie, txStream};
    UartRx uartRx{espUartPeripherie, rxStream};
};