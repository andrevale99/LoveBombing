#include "system.h"

#include "env.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "queue_task.h"
#include "uart_task.h"
#include "middleware_task.h"
#include "bomba_task.h"
#include "ads111x_task.h"

void system_init(void)
{
    queue_start();

    start_uart_task();
    start_middleware_task();
    start_bomba_task();
}

void start_uart_task(void)
{
    xTaskCreatePinnedToCore(task_uart, "uart_task",
                            ENV_TASK_UART_STACK_SIZE, NULL, ENV_TASK_UART_PRIORITY,
                            task_uart_get_handleTask(), ENV_CORE_0);
}

void start_middleware_task(void)
{
    xTaskCreatePinnedToCore(task_middleware, "middleware_task",
                            ENV_TASK_MIDDLEWARE_STACK_SIZE, NULL, ENV_TASK_MIDDLEWARE_PRIORITY,
                            task_middleware_get_handleTask(), ENV_CORE_0);
}

void start_bomba_task(void)
{
    xTaskCreatePinnedToCore(task_bomba, "bomba_task",
                            ENV_TASK_BOMBA_STACK_SIZE, NULL, ENV_TASK_BOMBA_PRIORITY,
                            task_bomba_get_handleTask(), ENV_CORE_0);
}

void start_ads111x_task(void)
{
    xTaskCreatePinnedToCore(task_ads111x, "adx111x_task",
                            ENV_TASK_ADS111X_STACK_SIZE, NULL, ENV_TASK_ADS111X_PRIORITY,
                            task_ads111x_get_handleTask(), ENV_CORE_0);
}