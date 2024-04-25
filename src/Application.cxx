#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "Application.hpp"
#include "wrappers/Task.hpp"
#include "core/SafeAssert.h"

#include <memory>

extern "C" void StartDefaultTask(void *) // NOLINT
{
    static auto app = std::make_unique<Application>();
    app->run();

    SafeAssert(false); // this line should be never reached
}

//--------------------------------------------------------------------------------------------------
Application::Application()
{
    // Delegated Singleton, see getApplicationInstance() for further explanations
    SafeAssert(instance == nullptr);
    instance = this;
}

//--------------------------------------------------------------------------------------------------
[[noreturn]] void Application::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
Application &Application::getApplicationInstance()
{
    // Not constructing Application in this singleton, to avoid bugs where something tries to
    // access this function, while application constructs which will cause infinite recursion
    return *instance;
}

//--------------------------------------------------------------------------------------------------
HAL_StatusTypeDef Application::registerCallbacks()
{
    HAL_StatusTypeDef result = HAL_OK;

    // SPI callback for addressable LEDs
    // result = HAL_SPI_RegisterCallback(LedSpiPeripherie, HAL_SPI_TX_COMPLETE_CB_ID,
    //                                [](SPI_HandleTypeDef *)
    //                              { getApplicationInstance().lightController.notifySpiIsFinished(); });

    return result;
}