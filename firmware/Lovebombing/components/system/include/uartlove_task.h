#ifndef uartlove_task_h
#define uartlove_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "uartlove_task";
TaskHandle_t handle_uartlove = NULL;

void task_uartlove(void *pvargs)
{
    ESP_LOGI(TAG, "uartlove task started");
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t* task_uartlove_get_handle(void)
{
    return &handle_uartlove;
}

#endif