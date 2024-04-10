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

    // TODO enable Buffer

    HAL_StatusTypeDef result = //
        HAL_SPI_Transmit_DMA(spiPeripherie, reinterpret_cast<uint8_t *>(&startFrame),
                             sizeof(startFrame));

    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));

    if (result != HAL_OK)
    {
        // ToDo: report error in a such way
        // but it is not super critical for system, so no further handlings
    }
}

//--------------------------------------------------------------------------------------------------
inline void AddressableLedDriver::convertToGammaCorrectedColors(LedSegmentArray &source,
                                                                LedSpiDataArray &destination)
{
    for (size_t i = 0; i < destination.size(); i++)
        destination[i].assignGammaCorrectedColor(source[i]);
}

//--------------------------------------------------------------------------------------------------
void AddressableLedDriver::sendBuffer(LedSegmentArray &ledSegmentArray)
{
    // TODO enable Buffer

    convertToGammaCorrectedColors(ledSegmentArray, ledSpiData);
    sendStartFrame();

    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_SPI_Transmit_DMA(spiPeripherie, reinterpret_cast<uint8_t *>(ledSpiData.data()),
                                  ledSpiData.size() * sizeof(LedSpiData));
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));

    result = HAL_SPI_Transmit_DMA(spiPeripherie, endFrames.data(), NumberOfEndFrames);
    ulTaskNotifyTake(pdTRUE, toOsTicks(Timeout));
    if (result != HAL_OK)
    {
        // ToDo: report error in a such way
        // but it is not super critical for system, so no further handlings
    }
}