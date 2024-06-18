#pragma once

#include <cstdint>

namespace response
{
constexpr auto ShiftPosition = 5;
constexpr auto Okay = 0x00;
constexpr auto OperationFailed = 0x01;
constexpr auto CrcFailed = 0x02;
constexpr auto NotSupported = 0x03;
constexpr auto InvalidPayloadSize = 0x04;
} // namespace response

constexpr auto ProtocolMagic = 0xBEEF;

// packet header
struct PacketHeader
{
    uint16_t topicLength = 0;
    uint8_t status = 0;
    uint16_t magic = ProtocolMagic;
    uint16_t payloadSize = 0;
} __attribute__((packed));