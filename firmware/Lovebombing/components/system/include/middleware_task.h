#ifndef MIDDLEWARE_TASK_H
#define MIDDLEWARE_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "env.h"
#include "cmd.h"

#include "esp_log.h"

static const char *TAG_MIDDLEWARE = "middleware_task";
TaskHandle_t handleTask_middleware = NULL;

void task_middleware(void *pvargs)
{
    ESP_LOGI(TAG_MIDDLEWARE, "middleware task started");

    cmd_t cmd = {0};

    while (1)
    {
        if (xQueueReceive(queue_uart_to_middleware, &cmd, -1) == pdTRUE)
        {
            ESP_LOGI(TAG_MIDDLEWARE, "%s: %d", cmd.cmd, cmd.value);

            if (strcmp(cmd.cmd, "S") == 0 || strcmp(cmd.cmd, "T") == 0)
                xQueueSend(queue_middleware_to_bomba, &cmd, -1);

            memset(&cmd, 0, sizeof(cmd_t));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

TaskHandle_t *task_middleware_get_handleTask(void)
{
    return &handleTask_middleware;
}

#endif