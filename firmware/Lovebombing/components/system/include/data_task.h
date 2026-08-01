#ifndef DATA_TASK
#define DATA_TASK

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"

#include "env.h"
#include "cmd.h"
#include "ads111x.h"

static const char *TAG_DATA = "data_task";
TaskHandle_t handleTask_data = NULL;

ads111x_struct_t ads = {0};
i2c_master_bus_handle_t handle_i2cmaster = NULL;

void task_data(void *pvargs)
{
    ESP_LOGI(TAG_MIDDLEWARE, "data task started");

    cmd_t cmd = {0};

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = ENV_SCL_GPIO,
        .sda_io_num = ENV_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &handle_i2cmaster));

    while (i2c_master_probe(handle_i2cmaster, ADS111X_ADDR, 100))
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGW(TAG_DATA, "ADS nao encontrado");
    }

    if (ads111x_begin(&handle_i2cmaster, ADS111X_ADDR, &ads) != ESP_OK)
        ESP_LOGE(TAG_DATA, "Erro ao iniciar o ADS111X");
    else
        ESP_LOGI(TAG_DATA, "ADS111X encontrado");

    ads111x_set_data_rate(ADS111X_DATA_RATE_475, &ads);
    ads111x_set_gain(ADS111X_GAIN_4V096, &ads);
    ads111x_set_mode(ADS111X_MODE_SINGLE_SHOT, &ads);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

TaskHandle_t *task_data_get_handleTask(void)
{
    return &handleTask_data;
}

#endif