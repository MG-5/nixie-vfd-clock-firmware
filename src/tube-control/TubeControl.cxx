#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

void TubeControl::taskMain(void *)
{
    vTaskDelay(toOsTicks(100.0_ms));
    enableBoostConverter.write(true);

    vTaskDelay(toOsTicks(100.0_ms));

    uint8_t tubeIndex = 0;
    uint8_t numberIndex = 0;
    uint32_t timeCounter = 0;

    while (1)
    {
        for (auto &tube : tubeArray)
            tube.write(false);

        for (auto &digit : digitArray)
            digit.write(false);

        tubeArray[tubeIndex].write(true);
        digitArray[numberIndex].write(true);

        if (++tubeIndex >= 6)
            tubeIndex = 0;

        if (++timeCounter >= 1000)
        {
            timeCounter = 0;

            if (++numberIndex >= 10)
                numberIndex = 0;
        }
        vTaskDelay(toOsTicks(1.0_ms));
    }
}
