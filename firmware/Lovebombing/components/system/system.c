#include "system.h"

#include "env.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "uartlove_task.h"

void system_init(void)
{
    tasks_start_uartlove_task();
}

void tasks_start_uartlove_task(void)
{
    xTaskCreatePinnedToCore(task_uartlove, "uartlove_task", 
        ENV_TASK_UARTLOVE_SIZE, NULL, ENV_TASK_UARTLOVE_PRIORITY, 
        task_uartlove_get_handle(), ENV_CORE_0);
}
