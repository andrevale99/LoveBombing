#include "heartbeat.h"

static const char *TAG = "heartbeat";

esp_err_t heartbeat_init(const heartbeat_t *heart)
{
    ESP_LOGI(TAG, "heartbeat install...");

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = HEARTBEAT_TIMER_DEFAULT,
        .freq_hz = 1 * 1000 * 10, // Set output frequency at 10 kHz
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_LOGI(TAG, "heartbeat timer config...");

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ESP_LOGI(TAG, "heartbeat timer config OK");

    if ((heart->gpioBomb_1[0] <= 0) || (heart->gpioBomb_1[1] <= 0) ||
        (heart->gpioBomb_2[0] <= 0) || (heart->gpioBomb_2[1] <= 0))
        {
            ESP_LOGE(TAG, "No GPIOS, please change it");
            return ESP_ERR_INVALID_ARG;
        }

        if (!(heart->gpioBomb_1[0] <= 0) || !(heart->gpioBomb_1[1] <= 0))
        {
            ESP_LOGI(TAG, "heartbeat channels %i,%i config install...",
                     heart->gpioBomb_1[0], heart->gpioBomb_1[1]);

            // Prepare and then apply the LEDC PWM channel configuration
            ledc_channel_config_t ledc_channel_1 = {
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = LEDC_CHANNEL_0,
                .timer_sel = 1,
                .intr_type = LEDC_INTR_DISABLE,
                .gpio_num = heart->gpioBomb_1[0],
                .duty = 0, // Set duty to 0%
                .hpoint = 0,
            };
            ESP_LOGI(TAG, "heartbeat channel config on gpio %i...", heart->gpioBomb_1[0]);
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_1));
            ESP_LOGI(TAG, "heartbeat channel config on gpio %i OK", heart->gpioBomb_1[0]);

            ledc_channel_config_t ledc_channel_2 = {
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = LEDC_CHANNEL_1,
                .timer_sel = 1,
                .intr_type = LEDC_INTR_DISABLE,
                .gpio_num = heart->gpioBomb_1[1],
                .duty = 0, // Set duty to 0%
                .hpoint = 0,
            };
            ESP_LOGI(TAG, "heartbeat channel config on gpio %i...", heart->gpioBomb_1[1]);
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_2));
            ESP_LOGI(TAG, "heartbeat channel config on gpio %i OK", heart->gpioBomb_1[1]);
        }

    if (!(heart->gpioBomb_2[0] <= 0) || !(heart->gpioBomb_2[1] <= 0))
    {
        ESP_LOGI(TAG, "heartbeat channels %i,%i config install...",
                 heart->gpioBomb_2[0], heart->gpioBomb_2[1]);

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel_1 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_2,
            .timer_sel = 1,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = heart->gpioBomb_2[0],
            .duty = 0, // Set duty to 0%
            .hpoint = 0,
        };

        ESP_LOGI(TAG, "heartbeat channel config on gpio %i...", heart->gpioBomb_2[0]);
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_1));
        ESP_LOGI(TAG, "heartbeat channel config on gpio %i OK", heart->gpioBomb_2[0]);

        ledc_channel_config_t ledc_channel_2 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_3,
            .timer_sel = 1,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = heart->gpioBomb_2[1],
            .duty = 0, // Set duty to 0%
            .hpoint = 0,
        };
        ESP_LOGI(TAG, "heartbeat channel config on gpio %i...", heart->gpioBomb_2[1]);
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_2));
        ESP_LOGI(TAG, "heartbeat channel config on gpio %i OK", heart->gpioBomb_2[1]);
    }

    ESP_LOGI(TAG, "heartbeat instaled");

    return ESP_OK;
}

esp_err_t heartbeat_deinit(const heartbeat_t *heart)
{
    ESP_LOGI(TAG, "heartbeat uninstall...");

    if (heart == NULL)
    {
        ESP_LOGE(TAG, "heart pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Desabilita os canais da bomba 1
    if ((heart->gpioBomb_1[0] > 0) && (heart->gpioBomb_1[1] > 0))
    {
        ESP_LOGI(TAG, "stopping channels on GPIO %d and %d",
                 heart->gpioBomb_1[0], heart->gpioBomb_1[1]);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
        gpio_reset_pin(heart->gpioBomb_1[0]);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0));
        gpio_reset_pin(heart->gpioBomb_1[1]);
    }

    // Desabilita os canais da bomba 2
    if ((heart->gpioBomb_2[0] > 0) && (heart->gpioBomb_2[1] > 0))
    {
        ESP_LOGI(TAG, "stopping channels on GPIO %d and %d",
                 heart->gpioBomb_2[0], heart->gpioBomb_2[1]);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0));
        gpio_reset_pin(heart->gpioBomb_2[0]);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, 0));
        gpio_reset_pin(heart->gpioBomb_2[1]);
    }

    ESP_LOGI(TAG, "heartbeat uninstalled");

    return ESP_OK;
}