#ifndef BOMBA_TASK_H
#define BOMBA_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "bomba.h"

#include "cmd.h"

static const char *TAG_BOMBA = "bomba_task";
TaskHandle_t handleTask_bomba = NULL;

const bomba_t bomba = {
    .gpioBomb_1[0] = 10,
    .gpioBomb_1[1] = 11,

    .gpioBomb_2[0] = -1,
    .gpioBomb_2[1] = -1,
};

void task_bomba(void *pvargs)
{
    ESP_LOGI(TAG_BOMBA, "bomba task started");

    bomba_init(&bomba);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    bomba_deinit(&bomba);
}

TaskHandle_t *task_bomba_get_handleTask(void)
{
    return &handleTask_bomba;
}

#endif