#include "AddressableLedDriver.hpp"

#include "FreeRTOS.h"
#include "helpers/freertos.hpp"
#include "task.h"

// see following links for implementation details
// https://cpldcpu.wordpress.com/2014/11/30/understanding-the-apa102-superled/
// https://cpldcpu.wordpress.com/2016/12/13/sk9822-a-clone-of-the-apa102/

//--------------------------------------------------------------------------------------------------
inline void AddressableLedDriver::sendStartFrame()
{
    uint32_t startFrame = 0;

    HAL_SPI_Transmit_IT(spiPeripherie, reinterpret_cast<uint8_t *>(&startFrame),
                        sizeof(startFrame));

    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));
}

//--------------------------------------------------------------------------------------------------
inline void AddressableLedDriver::applyBrightnessAndGammaCorrection(LedSegmentArray &source,
                                                                    LedSpiDataArray &destination)
{
    for (size_t i = 0; i < destination.size(); i++)
        destination[i].applyGammaCorrection(source[i] * brightnessPercentage);
}

//--------------------------------------------------------------------------------------------------
void AddressableLedDriver::sendBuffer(LedSegmentArray &ledSegmentArray)
{
    applyBrightnessAndGammaCorrection(ledSegmentArray, ledSpiData);
    sendStartFrame();

    HAL_SPI_Transmit_IT(spiPeripherie, reinterpret_cast<uint8_t *>(ledSpiData.data()),
                        ledSpiData.size() * sizeof(LedSpiData));
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));

    HAL_SPI_Transmit_IT(spiPeripherie, endFrames.data(), NumberOfEndFrames);
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));
}

//--------------------------------------------------------------------------------------------------
void AddressableLedDriver::sendZeroBuffer()
{
    sendStartFrame();

    HAL_SPI_Transmit_IT(spiPeripherie, reinterpret_cast<uint8_t *>(zeroData.data()),
                        zeroData.size() * sizeof(LedSpiData));
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));

    HAL_SPI_Transmit_IT(spiPeripherie, endFrames.data(), NumberOfEndFrames);
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));
}