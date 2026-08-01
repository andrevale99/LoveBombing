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

#define NUM_AMOSTRAS 100

static const char *TAG = "PRESSAO_TESTE";

//
// Variável global do PWM
// Valor permitido: 0 a 255
//
volatile uint32_t pwm_duty = 256*0.45;

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
    float p0Total;
    float offset0;
} SistemaData;

esp_err_t I2C_config(i2c_master_bus_handle_t *handle_I2C_master)
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 23,
        .sda_io_num = 22,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &i2c_mst_config,
            handle_I2C_master));

    return ESP_OK;
}

static void pwm_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = PWM_MODE,
        .timer_num = PWM_TIMER,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_cfg));

    ledc_channel_config_t channel_cfg = {
        .gpio_num = PWM_GPIO,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0};

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_cfg));
}

static void gptimer_init(void)
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000};

    ESP_ERROR_CHECK(
        gptimer_new_timer(
            &config,
            &gptimer));

    ESP_ERROR_CHECK(
        gptimer_enable(gptimer));

    ESP_ERROR_CHECK(
        gptimer_start(gptimer));
}

void set_offset_pressure(
    struct sistema_t *sistema,
    ads111x_struct_t *ads)
{
    float v_0 = 0.0f;

    sistema->offset0 = 0.0f;

    uint8_t cont = 0;
    float soma_p0 = 0.0f;

    for (cont = 0; cont < 100; ++cont)
    {
        ads111x_set_input_mux(
            ADS111X_MUX_0_GND,
            ads);

        ads111x_get_conversion_sigle_ended(ads);

        sistema->adc0 = ads->conversion;

        v_0 =
            (sistema->adc0 * 0.1875f) / 1000.0f;

        sistema->p0Total =
            (((v_0 / 5.0f) - 0.04f) / 0.018f);

        soma_p0 += sistema->p0Total;
    }

    sistema->offset0 =
        -(soma_p0 / 100.0f);
}

void process_pressures(struct sistema_t *sistema)
{
    float v_0 =
        (sistema->adc0 * 0.1875f) / 1000.0f;

    sistema->p0Total =
        (((v_0 / 5.0f) - 0.04f) / 0.018f);

    sistema->p0 =
        (((v_0 / 5.0f) - 0.04f) / 0.018f) + sistema->offset0;
}

void app_main(void)
{
    ads111x_struct_t ads;

    I2C_config(&handleI2Cmaster);

    while (
        i2c_master_probe(
            handleI2Cmaster,
            ADS111X_ADDR,
            100))
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));

        ESP_LOGW(
            TAG,
            "ADS nao encontrado");
    }

    if (
        ads111x_begin(
            &handleI2Cmaster,
            ADS111X_ADDR,
            &ads) != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Erro ao iniciar o ADS111X");
    }
    else
    {
        ESP_LOGI(
            TAG,
            "ADS111X encontrado");
    }

    ads111x_set_data_rate(
        ADS111X_DATA_RATE_475,
        &ads);

    ads111x_set_gain(
        ADS111X_GAIN_4V096,
        &ads);

    ads111x_set_mode(
        ADS111X_MODE_SINGLE_SHOT,
        &ads);

    set_offset_pressure(
        &SistemaData,
        &ads);

    ESP_LOGI(
        TAG,
        "Offset finalizado");

    pwm_init();

    gptimer_init();

    //
    // Varia o PWM de 30% até 100%
    // em passos de 5%
    //
    for (
        uint32_t pwm_percent = 30;
        pwm_percent <= 100;
        pwm_percent += 5)
    {
        //
        // Converte a porcentagem para duty de 8 bits
        //
        pwm_duty =
            (pwm_percent * PWM_MAX_DUTY) / 100;

        //
        // Aplica o novo PWM
        //
        ESP_ERROR_CHECK(
            ledc_set_duty(
                PWM_MODE,
                PWM_CHANNEL,
                pwm_duty));

        ESP_ERROR_CHECK(
            ledc_update_duty(
                PWM_MODE,
                PWM_CHANNEL));

        //
        // Aguarda a planta atingir o novo PWM
        //
        vTaskDelay(
            pdMS_TO_TICKS(1000));

        //
        // Variáveis para cálculo das médias
        //
        int64_t soma_adc0 = 0;

        float soma_p0 = 0.0f;

        uint64_t soma_tempo_aquisicao_us = 0;
        uint64_t soma_tempo_calculo_us = 0;
        uint64_t soma_tempo_total_us = 0;

        //
        // Aquisição de 100 amostras
        //
        for (
            uint32_t contador_amostras = 0;
            contador_amostras < NUM_AMOSTRAS;
            contador_amostras++)
        {
            uint64_t t0;
            uint64_t t1;
            uint64_t t2;

            //
            // Início da aquisição
            //
            gptimer_get_raw_count(
                gptimer,
                &t0);

            ads111x_set_input_mux(
                ADS111X_MUX_0_GND,
                &ads);

            ads111x_get_conversion_sigle_ended(
                &ads);

            SistemaData.adc0 =
                ads.conversion;

            //
            // Fim da aquisição
            //
            gptimer_get_raw_count(
                gptimer,
                &t1);

            //
            // Cálculo da pressão
            //
            process_pressures(
                &SistemaData);

            //
            // Fim do cálculo
            //
            gptimer_get_raw_count(
                gptimer,
                &t2);

            //
            // Tempos da amostra
            //
            tempo_aquisicao_us =
                t1 - t0;

            tempo_calculo_us =
                t2 - t1;

            tempo_total_us =
                t2 - t0;

            //
            // Acumula os dados
            //
            soma_adc0 +=
                SistemaData.adc0;

            soma_p0 +=
                SistemaData.p0;

            soma_tempo_aquisicao_us +=
                tempo_aquisicao_us;

            soma_tempo_calculo_us +=
                tempo_calculo_us;

            soma_tempo_total_us +=
                tempo_total_us;

            //
            // Intervalo entre as amostras
            //
            vTaskDelay(
                pdMS_TO_TICKS(10));
        }

        //
        // Calcula as médias
        //
        int16_t media_adc0 =
            soma_adc0 / NUM_AMOSTRAS;

        float media_p0 =
            soma_p0 / NUM_AMOSTRAS;

        uint64_t media_tempo_aquisicao_us =
            soma_tempo_aquisicao_us / NUM_AMOSTRAS;

        uint64_t media_tempo_calculo_us =
            soma_tempo_calculo_us / NUM_AMOSTRAS;

        uint64_t media_tempo_total_us =
            soma_tempo_total_us / NUM_AMOSTRAS;

        //
        // Exibe somente a média do passo atual
        //
        printf(
            "%lu;%d;%.3f;%" PRIu64 ";%" PRIu64 ";%" PRIu64 "\n",
            (unsigned long)pwm_percent,
            media_adc0,
            media_p0,
            media_tempo_aquisicao_us,
            media_tempo_calculo_us,
            media_tempo_total_us);
    }

    //
    // Desliga o PWM após o último ensaio
    //
    pwm_duty = 0;

    ESP_ERROR_CHECK(
        ledc_set_duty(
            PWM_MODE,
            PWM_CHANNEL,
            0));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            PWM_MODE,
            PWM_CHANNEL));

    //
    // Mantém a tarefa parada
    //
    while (1)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}