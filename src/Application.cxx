#include "FreeRTOS.h"
#include "main.h"
#include "stm32f1xx_hal_tim.h"
#include "task.h"

#include "Application.hpp"
#include "core/SafeAssert.h"
#include "wrappers/Task.hpp"

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

    registerCallbacks();
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
void Application::registerCallbacks()
{
    HAL_StatusTypeDef result = HAL_OK;

    // SPI callback for addressable LEDs
    result = HAL_SPI_RegisterCallback(
        LedSpiPeripherie, HAL_SPI_TX_COMPLETE_CB_ID, [](SPI_HandleTypeDef *)
        { getApplicationInstance().lightController.notifySpiIsFinished(); });
    SafeAssert(result == HAL_OK);

    // uart stuff
    result = HAL_UART_RegisterCallback(
        UartPeripherie, HAL_UART_TX_COMPLETE_CB_ID, [](UART_HandleTypeDef *)
        { getApplicationInstance().packetProcessor.uartTx.notifyTxTask(); });
    SafeAssert(result == HAL_OK);

    result = HAL_UART_RegisterCallback(
        UartPeripherie, HAL_UART_ERROR_CB_ID, [](UART_HandleTypeDef *)
        { getApplicationInstance().packetProcessor.uartRx.uartErrorFromISR(); });
    SafeAssert(result == HAL_OK);

    result = HAL_UART_RegisterRxEventCallback(
        UartPeripherie, [](UART_HandleTypeDef *, uint16_t pos)
        { getApplicationInstance().packetProcessor.uartRx.rxEventsFromISR(pos); });
    SafeAssert(result == HAL_OK);
}

//--------------------------------------------------------------------------------------------------
void Application::multiplexingTimerUpdate()
{
    getApplicationInstance().tubeControl.multiplexingTimerInterrupt();
}

//--------------------------------------------------------------------------------------------------
void Application::pwmTimerCompare()
{
    getApplicationInstance().tubeControl.pwmTimerInterrupt();
}

void Application::clockSecondTimeout(TimerHandle_t)
{
    getApplicationInstance().clock.timeoutInterrupt();
}

void Application::fadingTimerCompare()
{
    getApplicationInstance().tubeControl.fadingTimerInterrupt();
}

//--------------------------------------------------------------------------------------------------
// skip HAL`s interupt routine to get more performance
extern "C" void TIM1_UP_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(Application::MultiplexingPwmTimer, TIM_IT_UPDATE);
    Application::multiplexingTimerUpdate();
}

extern "C" void TIM1_CC_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(Application::MultiplexingPwmTimer, TIM_FLAG_CC1) == SET)
    {
        if (__HAL_TIM_GET_IT_SOURCE(Application::MultiplexingPwmTimer, TIM_IT_CC1) == SET)
        {
            __HAL_TIM_CLEAR_IT(Application::MultiplexingPwmTimer, TIM_IT_CC1);
            Application::pwmTimerCompare();
        }
    }

    if (__HAL_TIM_GET_FLAG(Application::MultiplexingPwmTimer, TIM_FLAG_CC2) == true)
    {
        if (__HAL_TIM_GET_IT_SOURCE(Application::MultiplexingPwmTimer, TIM_IT_CC2) == SET)
        {
            __HAL_TIM_CLEAR_IT(Application::MultiplexingPwmTimer, TIM_IT_CC2);
            Application::fadingTimerCompare();
        }
    }
}