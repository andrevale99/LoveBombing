#ifndef DATA_TASK
#define DATA_TASK

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "env.h"
#include "cmd.h"
#include "ads111x.h"

#include "esp_log.h"

static const char *TAG_DATA = "data_task";
TaskHandle_t handleTask_data = NULL;

void task_data(void *pvargs)
{
    ESP_LOGI(TAG_MIDDLEWARE, "data task started");

    cmd_t cmd = {0};

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

TaskHandle_t *task_data_get_handleTask(void)
{
    return &handleTask_data;
}

#endif