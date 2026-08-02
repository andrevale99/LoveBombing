#ifndef DATA_TASK
#define DATA_TASK

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"

#include "env.h"
#include "cmd.h"
#include "ads111x.h"

#define Apwm 18.84928
#define Bpwm 0.052304
#define Cpwm 1.045616

#define Avazao 4.265640
#define Bvazao 0.719007
#define Cvazao 0.254911

#define DUTY_TO_PERCENT(_duty) ((float)_duty / 256.f * 100.f)

static const char *TAG_DATA = "data_task";
TaskHandle_t handleTask_data = NULL;

ads111x_struct_t ads = {0};
i2c_master_bus_handle_t handle_i2cmaster = NULL;

void set_offset_pressure(data_t *data, ads111x_struct_t *ads)
{
    float v_0 = 0.0;
    uint8_t cont = 0;
    float soma_p0 = 0;

    data->offset = 0.0;

    for (cont = 0; cont < 100; ++cont)
    {
        ads111x_set_input_mux(ADS111X_MUX_0_GND, ads);
        ads111x_get_conversion_sigle_ended(ads);
        data->adc = ads->conversion;

        v_0 = (data->adc * 0.1875) / 1000;

        data->pressaoTotal = (((v_0 / 5) - 0.04) / 0.018);

        soma_p0 += data->pressaoTotal;
    }

    data->offset = -(soma_p0 / 100);
}

void process_pressures(data_t *data)
{
    float v_0 = (data->adc * 0.1875) / 1000;

    data->pressaoTotal = (((v_0 / 5) - 0.04) / 0.018);

    data->pressao = data->pressaoTotal + data->offset;
}

void task_data(void *pvargs)
{
    ESP_LOGI(TAG_MIDDLEWARE, "data task started");

    int flagDuty = 0;
    data_t data = {0};

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

    ads111x_set_data_rate(ADS111X_DATA_RATE_860, &ads);
    ads111x_set_gain(ADS111X_GAIN_4V096, &ads);
    ads111x_set_mode(ADS111X_MODE_SINGLE_SHOT, &ads);

    set_offset_pressure(&data, &ads);

    while (1)
    {
        if (xQueueReceive(queue_bomba_to_data, &flagDuty, 0) == pdTRUE)
            if (flagDuty >= 0)
                data.duty = flagDuty;

        float pulso_ms = Apwm * expf(-Bpwm * DUTY_TO_PERCENT(data.duty)) + Cpwm;
        data.vazao = Avazao * exp(-Bvazao * pulso_ms) + Cvazao;

        if (DUTY_TO_PERCENT(data.duty) < 28 )
            data.vazao = 0;

        ads111x_set_input_mux(ADS111X_MUX_0_GND, &ads);
        ads111x_get_conversion_sigle_ended(&ads);
        data.adc = ads.conversion;

        process_pressures(&data);

        xQueueSend(queue_data_to_uart, &data, 0);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

TaskHandle_t *task_data_get_handleTask(void)
{
    return &handleTask_data;
}

#endif