#include "LabMecFluPWM.h"

static QueueHandle_t *handleUART_to_PWM;

static const char *TAG = "[LabMecFluPWM]";

esp_err_t LabMecFluPWM_config(QueueHandle_t *handleQueue)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = 1,
        .freq_hz = 1 * 1000 * 10, // Set output frequency at 10 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = 1,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BOMBA_GPIO,
        .duty = 0, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    handleUART_to_PWM = handleQueue;

    xTaskCreatePinnedToCore(vTaskPWM,
                            "vTaskPWM",
                            2048,
                            NULL,
                            2,
                            NULL,
                            1); // Pin the task to core 1

    return ESP_OK;
}

void vTaskPWM(void *pvParameters)
{
    uint32_t duty = 0;
    LabMecFluUART_Command_t command;
    TickType_t lastWake;
    const TickType_t periodo = pdMS_TO_TICKS(2);
    while (1)
    {
        lastWake = xTaskGetTickCount();
        if (xQueueReceive(*handleUART_to_PWM, &command,
                          0) == pdPASS)
        {
            if (command.cmd == 'S')
            {
                if (command.value < 0)
                    duty = 0;
                else if (command.value > POTENCIA_MAX_BOMBA)
                    duty = POTENCIA_MAX_BOMBA;
                else
                    duty = command.value;

                duty = (duty / 100.) * 255;

                ESP_LOGI(TAG, "Ajustando duty cycle para: %d", duty);

                // Set the duty cycle
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE,
                                              LEDC_CHANNEL_0,
                                              duty));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE,
                                                 LEDC_CHANNEL_0));
            }
            else if (command.cmd == 'T')
            {
                // Handle other commands if necessary
            }
        }
        vTaskDelayUntil(&lastWake, periodo);
    }
}