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
    xTaskCreate(task_uartlove, "uartlove_task", 
        TASK_UARTLOVE_SIZE, NULL, TASK_UARTLOVE_PRIORITY, 
        task_uartlove_get_handle());
}
