#include "bomba.h"

static const char *TAG = "bomba";

esp_err_t bomba_init(const bomba_t *heart)
{
    ESP_LOGI(TAG, "bomba install...");

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = BOMBA_TIMER_DEFAULT,
        .freq_hz = 1 * 1000 * 10, // Set output frequency at 10 kHz
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_LOGI(TAG, "bomba timer config...");

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ESP_LOGI(TAG, "bomba timer config OK");

    if (!(heart->gpioBomb_1 <= 0))
    {
        ESP_LOGI(TAG, "bomba channels %i config install...",
                 heart->gpioBomb_1);

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel_1 = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = heart->channel_gpiobomb1,
            .timer_sel = 1,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = heart->gpioBomb_1,
            .duty = 0, // Set duty to 0%
            .hpoint = 0,
        };
        ESP_LOGI(TAG, "bomba channel config on gpio %i...", heart->gpioBomb_1);
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_1));
        ESP_LOGI(TAG, "bomba channel config on gpio %i OK", heart->gpioBomb_1);
    }

    if (!(heart->gpioBomb_2 <= 0))
    {
        ESP_LOGI(TAG, "bomba channels %i config install...",
                 heart->gpioBomb_2);

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel_1 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = heart->channel_gpiobomb1,
            .timer_sel = 1,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = heart->gpioBomb_2,
            .duty = 0, // Set duty to 0%
            .hpoint = 0,
        };

        ESP_LOGI(TAG, "bomba channel config on gpio %i...", heart->gpioBomb_2);
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_1));
        ESP_LOGI(TAG, "bomba channel config on gpio %i OK", heart->gpioBomb_2);
    }

    ESP_LOGI(TAG, "bomba instaled");

    return ESP_OK;
}

esp_err_t bomba_deinit(const bomba_t *heart)
{
    ESP_LOGI(TAG, "bomba uninstall...");

    if (heart == NULL)
    {
        ESP_LOGE(TAG, "heart pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Desabilita os canais da bomba 1
    if ((heart->gpioBomb_1 > 0))
    {
        ESP_LOGI(TAG, "stopping channels on GPIO %d",
                 heart->gpioBomb_1);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
        gpio_reset_pin(heart->gpioBomb_1);
    }

    // Desabilita os canais da bomba 2
    if ((heart->gpioBomb_2 > 0))
    {
        ESP_LOGI(TAG, "stopping channels on GPIO %d",
                 heart->gpioBomb_2);

        ESP_ERROR_CHECK(
            ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0));
        gpio_reset_pin(heart->gpioBomb_2);
    }

    ESP_LOGI(TAG, "bomba uninstalled");

    return ESP_OK;
}

esp_err_t bomba_set_duty(const bomba_t *bomba, which_bomb_t select, int duty)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;

    if (!bomba)
        return ret;

    switch (select)
    {
    case BOMB_1:
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb1, duty);
        ret = ledc_update_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb1);
        break;

    case BOMB_2:
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb2, duty);
        ret = ledc_update_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb2);
        break;

    default:
        ret = ESP_ERR_NOT_FOUND;
        ESP_LOGE(TAG, "Bomb not found");
        break;
    }

    return ret;
}

int bomba_get_actualduty(const bomba_t *bomba, which_bomb_t select)
{
    if (!bomba)
        return ESP_ERR_NOT_FOUND;

    if (select < BOMB_1 || select > BOMB_2)
        return ESP_ERR_NOT_FOUND;

    if (select == BOMB_1)
        return ledc_get_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb1);

    else
        return ledc_get_duty(LEDC_HIGH_SPEED_MODE, bomba->channel_gpiobomb2);

    return -1;
}