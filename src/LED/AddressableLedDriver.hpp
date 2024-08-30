#pragma once

#include "LedDataTypes.hpp"
#include "core/SafeAssert.h"
#include "units/si/time.hpp"
#include "util/led/GammaCorrection.hpp"

#include "stm32f1xx_hal.h"

// Controls syncronous addressable LEDs (APA102,SK9822) over SPI hardware
class AddressableLedDriver
{
public:
    static constexpr auto Timeout = 2.0_s;

    explicit AddressableLedDriver(SPI_HandleTypeDef *spiPeripherie) : spiPeripherie(spiPeripherie)
    {
        SafeAssert(spiPeripherie != nullptr);
        endFrames.fill(0xFF);
    };

    static constexpr auto LedPwmResolutionBits = 8;

    void sendBuffer(LedSegmentArray &ledSegmentArray);

private:
    SPI_HandleTypeDef *spiPeripherie = nullptr;

    struct LedSpiData
    {
        uint8_t Start = 0xFF; //!< the first byte contains control data like brightness
        BgrColor color;

        void applyGammaCorrection(BgrColor newColor)
        {
            color.blue = GammaCorrection.LookUpTable[newColor.blue];
            color.green = GammaCorrection.LookUpTable[newColor.green];
            color.red = GammaCorrection.LookUpTable[newColor.red];
        }
    };
    using LedSpiDataArray = std::array<LedSpiData, NumberOfLeds>;

    LedSpiDataArray ledSpiData;

    static constexpr auto NumberOfEndFrames = (NumberOfLeds + 15) / 16;
    std::array<uint8_t, NumberOfEndFrames> endFrames{};

    static constexpr util::led::pwm::GammaCorrection<LedPwmResolutionBits> GammaCorrection{};

    void sendStartFrame();

    /// convert LED data to gamma corrected colors and put it to SPI-related array
    void convertToGammaCorrectedColors(LedSegmentArray &source, LedSpiDataArray &destination);
};