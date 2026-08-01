#ifndef BOMBA_TASK_H
#define BOMBA_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "bomba.h"

#include "cmd.h"

static const char *TAG_BOMBA = "bomba_task";
TaskHandle_t handleTask_bomba = NULL;

bomba_t bomba = {
    .gpioBomb_1 = 13,

    .gpioBomb_2 = -1,
};

void task_bomba(void *pvargs)
{
    ESP_LOGI(TAG_BOMBA, "bomba task started");

    cmd_t _cmd = {0};
    int duty = 0;

    bomba_init(&bomba);

    while (1)
    {
        if (xQueueReceive(queue_middleware_to_bomba, &_cmd, -1))
        {
            if (strcmp(_cmd.cmd, "S") == 0)
            {
                duty = ((float)_cmd.value) / 100. * 256.;

                if (duty > 255)
                    duty = 254;
                else if (duty < 0)
                    duty = 0;

                ESP_LOGI(TAG_BOMBA, "Novo duty: %i", duty);

                ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            }
            else if (strcmp(_cmd.cmd, "X") == 0)
            {
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    bomba_deinit(&bomba);
}

TaskHandle_t *task_bomba_get_handleTask(void)
{
    return &handleTask_bomba;
}

#endif