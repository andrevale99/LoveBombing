#ifndef BOMBA_TASK_H
#define BOMBA_TASK_H

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "bomba.h"

#include "cmd.h"

static const char *TAG_BOMBA = "bomba_task";
TaskHandle_t handleTask_bomba = NULL;

bomba_t bomba = {
    .gpioBomb_1 = 13,
    .channel_gpiobomb1 = LEDC_CHANNEL_0,

    .gpioBomb_2 = -1,
    .channel_gpiobomb2 = -1,
};

static TickType_t xLastWakeTime;

#define LEN_SINAL_AD 201
int sinal_AD[LEN_SINAL_AD] = {142, 142, 142, 142, 143, 144, 145,
                              146, 147, 148, 150, 152, 153, 155, 157,
                              159, 160, 162, 164, 165, 167, 168, 170,
                              171, 173, 174, 175, 176, 177, 178, 178,
                              179, 179, 180, 180, 180, 180, 180, 180,
                              180, 179, 179, 179, 179, 178, 178, 177,
                              177, 177, 177, 177, 177, 176, 175, 173,
                              172, 171, 170, 169, 168, 167, 166, 165,
                              164, 163, 162, 161, 161, 160, 159, 158,
                              158, 157, 156, 155, 155, 154, 154, 153,
                              152, 152, 151, 151, 150, 150, 150, 149,
                              149, 149, 150, 151, 153, 154, 155, 157,
                              158, 159, 161, 162, 164, 165, 164, 163,
                              162, 162, 161, 160, 159, 159, 158, 157,
                              156, 156, 155, 154, 154, 153, 152, 152,
                              151, 150, 150, 149, 149, 148, 147, 147,
                              146, 146, 145, 145, 144, 144, 143, 143,
                              142, 142, 141, 141, 140, 140, 139, 139,
                              138, 138, 138, 137, 137, 136, 136, 136,
                              135, 135, 135, 134, 134, 134, 134, 133,
                              133, 133, 133, 132, 132, 132, 132, 131,
                              131, 131, 131, 131, 131, 130, 130, 130,
                              130, 130, 130, 130, 130, 130, 129, 129,
                              129, 129, 129, 129, 129, 129, 129, 129,
                              129, 129, 129, 129, 130, 130, 130, 130,
                              130, 130};

void task_bomba(void *pvargs)
{
    ESP_LOGI(TAG_BOMBA, "bomba task started");

    cmd_t _cmd = {0};
    int duty = 0;

    const TickType_t xPeriodo = pdMS_TO_TICKS(5);
    xLastWakeTime = xTaskGetTickCount();

    bomba_init(&bomba);

    int cnt = 0;
    int idx = 0;

    while (1)
    {
        cnt++;

        if (cnt >= 10)
        {
            cnt = 0;
            idx = (idx + 1) % LEN_SINAL_AD;
        }

        bomba_set_duty(&bomba, BOMB_1, sinal_AD[idx]);

        xQueueSend(queue_bomba_to_data, &sinal_AD[idx], portMAX_DELAY);

        if (xQueueReceive(queue_middleware_to_bomba, &_cmd, 0))
        {
            if (strcmp(_cmd.cmd, "S") == 0)
            {
                duty = ((float)_cmd.value) / 100. * 256.;

                if (duty > 255)
                    duty = 254;
                else if (duty < 0)
                    duty = 0;

                // ESP_LOGI(TAG_BOMBA, "Novo duty: %i", duty);

                bomba_set_duty(&bomba, BOMB_1, duty);

                xQueueSend(queue_bomba_to_data, &duty, -1);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriodo);
    }

    bomba_deinit(&bomba);
}

TaskHandle_t *task_bomba_get_handleTask(void)
{
    return &handleTask_bomba;
}

#endif