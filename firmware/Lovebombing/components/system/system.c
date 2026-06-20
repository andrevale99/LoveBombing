#include "system.h"

#include "env.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "queue_task.h"
#include "uartlove_task.h"
#include "mailman_task.h"

void system_init(void)
{
    queue_start();

    start_uartlove_task();
    start_mailman_task();
}

void start_uartlove_task(void)
{
    xTaskCreatePinnedToCore(task_uartlove, "uartlove_task",
                            ENV_TASK_UARTLOVE_STACK_SIZE, NULL, ENV_TASK_UARTLOVE_PRIORITY,
                            task_uartlove_get_handle(), ENV_CORE_0);
}

void start_mailman_task(void)
{
    xTaskCreatePinnedToCore(task_mailman, "mailman_task",
                            ENV_TASK_MAILMAN_STACK_SIZE, NULL, ENV_TASK_MAILMAN_PRIORITY,
                            task_mailman_get_handle(), ENV_CORE_0);
}
