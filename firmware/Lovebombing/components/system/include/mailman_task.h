#ifndef mailman_task_h
#define mailman_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "env.h"
#include "letter.h"

#include "esp_log.h"

static const char *TAG_MAILMAN = "mailman_task";
TaskHandle_t handle_mailman = NULL;

void task_mailman(void *pvargs)
{
    ESP_LOGI(TAG_MAILMAN, "maiilman task started");

    letter_t letter = {0};

    while (1)
    {
        if (xQueueReceive(queue_uartlove_to_mailman, &letter, -1) == pdTRUE)
        {
            ESP_LOGI(TAG_MAILMAN, "%s: %d", letter.cmd, letter.value);
            memset(&letter, 0, sizeof(letter_t));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

TaskHandle_t *task_mailman_get_handle(void)
{
    return &handle_mailman;
}

#endif