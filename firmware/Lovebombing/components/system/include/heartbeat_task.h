#ifndef heartbeat_task_h
#define heartbeat_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "heartbeat.h"

#include "letter.h"

static const char *TAG_HEARTBEAT = "heartbeat_task";
TaskHandle_t handle_heartbeat = NULL;

const heartbeat_t heart = {
    .gpioBomb_1[0] = 10,
    .gpioBomb_1[1] = 11,

    .gpioBomb_2[0] = -1,
    .gpioBomb_2[1] = -1,
};

void task_heartbeat(void *pvargs)
{
    ESP_LOGI(TAG_HEARTBEAT, "heatbeat task started");

    heartbeat_init(&heart);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    heartbeat_deinit(&heart);
}

TaskHandle_t *task_heartbeat_get_handle(void)
{
    return &handle_heartbeat;
}

#endif