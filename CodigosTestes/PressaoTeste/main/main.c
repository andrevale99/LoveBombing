/*
 * PWM + GPTimer
 * ESP-IDF v6
 *
 * PWM:
 *  - LEDC
 *  - 8 bits
 *  - Duty controlado por variável global
 *
 * Timer:
 *  - GPTimer
 *  - Resolução de 1 us
 *  - Tempo armazenado em variável global
 */

#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "driver/gptimer.h"

#include "esp_err.h"
#include "esp_log.h"

#include "ads111x.h"

#define PWM_GPIO GPIO_NUM_13
#define PWM_FREQ_HZ 5000

#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0
#define PWM_CHANNEL LEDC_CHANNEL_0

#define PWM_RESOLUTION LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255

static const char *TAG = "PRESSAO_TESTE";

//
// Variável global do PWM
// Valor permitido: 0 a 255
//
volatile uint32_t pwm_duty = 0;

//
// Tempos de processamento (us)
//
volatile uint64_t tempo_aquisicao_us = 0;
volatile uint64_t tempo_calculo_us = 0;
volatile uint64_t tempo_total_us = 0;

//
// Variável global do tempo (us)
//
volatile uint64_t tempo_us = 0;

//
// Handle do GPTimer
//
static gptimer_handle_t gptimer = NULL;

i2c_master_bus_handle_t handleI2Cmaster = NULL;

struct sistema_t
{
    int16_t adc0;
    float p0;
    float p0Total; // Mais a coluna D'agua
    float offset0;
} SistemaData;

static void pwm_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = PWM_MODE,
        .timer_num = PWM_TIMER,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t channel_cfg = {
        .gpio_num = PWM_GPIO,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&channel_cfg));
}

static void gptimer_init(void)
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000 // 1 MHz -> 1 tick = 1 us
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer));

    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void set_offset_pressure(struct sistema_t *sistema, ads111x_struct_t *ads)
{
    float v_0 = 0.0;

    sistema->offset0 = 0.0;

    uint8_t cont = 0;
    float soma_p0 = 0;

    for (cont = 0; cont < 100; ++cont)
    {
        ads111x_set_input_mux(ADS111X_MUX_0_GND, ads);
        ads111x_get_conversion_sigle_ended(ads);
        sistema->adc0 = ads->conversion;

        v_0 = (sistema->adc0 * 0.1875) / 1000;

        sistema->p0Total = (((v_0 / 5) - 0.04) / 0.018);

        soma_p0 += sistema->p0Total;
    }

    sistema->offset0 = -(soma_p0 / 100);
}

void process_pressures(struct sistema_t *sistema)
{
    float v_0 = (sistema->adc0 * 0.1875) / 1000;

    sistema->p0Total = (((v_0 / 5) - 0.04) / 0.018);

    sistema->p0 = (((v_0 / 5) - 0.04) / 0.018) + sistema->offset0;
}

void app_main(void)
{
    ads111x_struct_t ads;

    while (i2c_master_probe(handleI2Cmaster, ADS111X_ADDR, 100))
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGW(TAG, "ADS nao encontrado");
    }

    if (ads111x_begin(&handleI2Cmaster, ADS111X_ADDR, &ads) != ESP_OK)
        ESP_LOGE(TAG, "Erro ao iniciar o ADS111X");
    else
        ESP_LOGI(TAG, "ADS111X encontrado");

    ads111x_set_data_rate(ADS111X_DATA_RATE_32, &ads);
    ads111x_set_gain(ADS111X_GAIN_4V096, &ads);
    ads111x_set_mode(ADS111X_MODE_SINGLE_SHOT, &ads);

    set_offset_pressure(&SistemaData, &ads);
    ESP_LOGI(TAG, "Offset finalizado");

    pwm_init();

    gptimer_init();

    uint32_t last_duty = 0xFFFFFFFF;

    while (1)
    {
        //
        // Atualiza o tempo global
        //
        ESP_ERROR_CHECK(
            gptimer_get_raw_count(
                gptimer,
                (uint64_t *)&tempo_us));

        //
        // Atualiza o PWM apenas quando houver alteração
        //
        if (last_duty != pwm_duty)
        {
            if (pwm_duty > PWM_MAX_DUTY)
                pwm_duty = PWM_MAX_DUTY;

            ledc_set_duty(
                PWM_MODE,
                PWM_CHANNEL,
                pwm_duty);

            ledc_update_duty(
                PWM_MODE,
                PWM_CHANNEL);

            last_duty = pwm_duty;
        }

        uint64_t t0, t1, t2;

        //
        // Início da aquisição
        //
        gptimer_get_raw_count(gptimer, &t0);

        ads111x_set_input_mux(ADS111X_MUX_0_GND, &ads);
        ads111x_get_conversion_sigle_ended(&ads);
        SistemaData.adc0 = ads.conversion;

        //
        // Fim da aquisição
        //
        gptimer_get_raw_count(gptimer, &t1);

        //
        // Cálculo da pressão
        //
        process_pressures(&SistemaData);

        //
        // Fim do cálculo
        //
        gptimer_get_raw_count(gptimer, &t2);

        tempo_aquisicao_us = t1 - t0;
        tempo_calculo_us = t2 - t1;
        tempo_total_us = t2 - t0;

        printf("%d,%.3f,%" PRIu32 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n",
               SistemaData.adc0,
               SistemaData.p0,
               pwm_duty,
               tempo_aquisicao_us,
               tempo_calculo_us,
               tempo_total_us);
               
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}