#ifndef ADS111X_TASK_H
#define ADS111X_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ads111x.h"

#include "esp_log.h"

static char *TAG_ADS111X = "ads111x_task";
TaskHandle_t handleTask_ads111x = NULL;

static ads111x_struct_t ads = {0};
static i2c_master_bus_handle_t handleI2C_master = NULL;

esp_err_t I2C_config(i2c_master_bus_handle_t *handle_I2C_master)
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = ENV_I2C_SCL_PIN,
        .sda_io_num = ENV_I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, handle_I2C_master));

    return ESP_OK;
}

void task_ads111x(void *pvargs)
{
    ESP_LOGI(TAG_ADS111X, "ads111x task started");

    I2C_config(&handleI2C_master);

    while (i2c_master_probe(handleI2C_master, ADS111X_ADDR, 100))
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGW(TAG_ADS111X, "ADS nao encontrado");
    }

    if (ads111x_begin(&handleI2C_master, ADS111X_ADDR, &ads) != ESP_OK)
        ESP_LOGE(TAG_ADS111X, "Erro ao iniciar o ADS111X");
    else
        ESP_LOGI(TAG_ADS111X, "ADS111X encontrado");

    ads111x_set_data_rate(ADS111X_DATA_RATE_64, &ads);
    ads111x_set_gain(ADS111X_GAIN_4V096, &ads);
    ads111x_set_mode(ADS111X_MODE_SINGLE_SHOT, &ads);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t *task_ads111x_get_handleTask(void)
{
    return &handleTask_ads111x;
}

#endif