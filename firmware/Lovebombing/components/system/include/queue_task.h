#ifndef queue_task_h
#define queue_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "cmd.h"

QueueHandle_t queue_uart_to_middleware = NULL;

void queue_start(void)
{
    queue_uart_to_middleware = xQueueCreate(3, sizeof(cmd_t));
}

#endif