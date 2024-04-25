#include "TubeControl.hpp"
#include "helpers/freertos.hpp"

void TubeControl::taskMain(void *)
{
    // wait for steady voltages
    vTaskDelay(toOsTicks(100.0_ms));
    tubes->setup();
    vTaskDelay(toOsTicks(100.0_ms));

    // tubes->setClock(18, 42, 05);

    uint16_t counter = 0;
    uint8_t number = 0;
    bool enableDots = false;

    while (1)
    {
        tubes->multiplexingStep();
        vTaskDelay(toOsTicks(1.0_ms));

        if (++counter >= 1000)
        {
            counter = 0;
            if (++number >= 10)
                number = 0;

            const auto numberToShow = number * 10 + number;
            tubes->setClock(numberToShow, numberToShow, numberToShow);
            enableDots = !enableDots;
            tubes->enableDots(enableDots);
        }
    }
}
