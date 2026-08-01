#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "driver/gptimer.h"
#include "esp_adc/adc_oneshot.h"

#include "esp_err.h"
#include "esp_log.h"

#include "ads111x.h"

#include "esp_timer.h"

#define PWM_GPIO GPIO_NUM_13
#define PWM_FREQ_HZ 5000

#define PWM_MODE LEDC_HIGH_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0
#define PWM_CHANNEL LEDC_CHANNEL_0

#define PWM_RESOLUTION LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255

#define LEN_SINAL_AD 201
#define TEMPO_TOTAL_MS 800
#define SAMPLE_PERIOD_US ((uint64_t)((TEMPO_TOTAL_MS * 1000.0f) / LEN_SINAL_AD + 0.5f))

#define NUM_REPETICOES 3
#define TOTAL_AMOSTRAS (LEN_SINAL_AD * NUM_REPETICOES)

#define ADC_ATTENUATION ADC_ATTEN_DB_12
#define ADC_BIT_WIDTH ADC_BITWIDTH_10

const char *TAG = "PressaTransiente";

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

i2c_master_bus_handle_t handleI2Cmaster = NULL;

gptimer_handle_t gptimer = NULL;

int adc_gpio14;

volatile uint16_t indice = 0;
volatile bool terminou = false;

ads111x_struct_t ads;

struct sistema_t
{
    int16_t adc0;
    float p0;
    float p0Total;
    float offset0;
} SistemaData;

typedef struct
{
    uint32_t tempo_us; // Tempo desde o início da aquisição (µs)

    int16_t sinal_ad; // Valor do vetor sinal_AD

    int16_t adc_ads;    // Conversão bruta do ADS1115
    int16_t adc_gpio14; // Conversão do ADC do GPIO14

    float pressao;       // Pressão compensada (kPa)
    float pressao_total; // Pressão sem offset (kPa)

} amostra_t;

amostra_t amostras[TOTAL_AMOSTRAS];

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

static bool IRAM_ATTR timer_callback(
    gptimer_handle_t timer,
    const gptimer_alarm_event_data_t *edata,
    void *user_ctx)
{
    if (indice < LEN_SINAL_AD)
    {
        ads111x_set_input_mux(
            ADS111X_MUX_0_GND,
            &ads);

        ads111x_get_conversion_sigle_ended(
            &ads);

        process_pressures(&SistemaData);

        SistemaData.adc0 = ads.conversion;

        amostras[indice].tempo_us = indice * SAMPLE_PERIOD_US;
        amostras[indice].sinal_ad = sinal_AD[indice];
        amostras[indice].adc_ads = SistemaData.adc0;
        amostras[indice].pressao = SistemaData.p0;
        amostras[indice].pressao_total = SistemaData.p0Total;

        indice++;
    }
    else
    {
        terminou = true;
        return false; // para gerar interrupções
    }

    return true;
}

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
        .resolution_hz = 1000000, // 1 us
    };

    ESP_ERROR_CHECK(
        gptimer_new_timer(
            &config,
            &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };

    ESP_ERROR_CHECK(
        gptimer_register_event_callbacks(
            gptimer,
            &cbs,
            NULL));

    gptimer_alarm_config_t alarm = {
        .alarm_count = SAMPLE_PERIOD_US,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    ESP_ERROR_CHECK(
        gptimer_set_alarm_action(
            gptimer,
            &alarm));

    ESP_ERROR_CHECK(
        gptimer_enable(gptimer));
}

static adc_oneshot_unit_handle_t adc2_handle;

static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_cfg,
            &adc2_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BIT_WIDTH,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc2_handle,
            ADC_CHANNEL_6,
            &chan_cfg));
}

void app_main(void)
{
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
        ADS111X_DATA_RATE_32,
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

    // gptimer_init();

    indice = 0;
    terminou = false;

    // ESP_ERROR_CHECK(
    //     gptimer_start(gptimer));

    adc_init();

    int64_t tempo_inicio = esp_timer_get_time();

    for (int vzs = 0; vzs < NUM_REPETICOES; vzs++)
    {
        for (uint16_t i = 0; i < LEN_SINAL_AD; i++)
        {
            /* Aguarda o instante da próxima amostra */
            int64_t instante = tempo_inicio + (int64_t)i * SAMPLE_PERIOD_US;

            while (esp_timer_get_time() < instante)
            {
                taskYIELD();
            }

            /* Atualiza o PWM */
            ESP_ERROR_CHECK(
                ledc_set_duty(
                    PWM_MODE,
                    PWM_CHANNEL,
                    sinal_AD[i]));

            ESP_ERROR_CHECK(
                ledc_update_duty(
                    PWM_MODE,
                    PWM_CHANNEL));

            /* Leitura do ADS1115 */
            ads111x_set_input_mux(
                ADS111X_MUX_0_GND,
                &ads);

            ads111x_get_conversion_sigle_ended(&ads);

            SistemaData.adc0 = ads.conversion;

            process_pressures(&SistemaData);

            ESP_ERROR_CHECK(
                adc_oneshot_read(
                    adc2_handle,
                    ADC_CHANNEL_3,
                    &adc_gpio14));

            uint16_t idx = vzs * LEN_SINAL_AD + i;

            amostras[idx].tempo_us = esp_timer_get_time();
            amostras[idx].sinal_ad = sinal_AD[i];
            amostras[idx].adc_ads = SistemaData.adc0;
            amostras[idx].adc_gpio14 = adc_gpio14;
            amostras[idx].pressao = SistemaData.p0;
            amostras[idx].pressao_total = SistemaData.p0Total;
        }

        /* Desliga o PWM entre pulsos */
        ESP_ERROR_CHECK(
            ledc_set_duty(
                PWM_MODE,
                PWM_CHANNEL,
                0));

        ESP_ERROR_CHECK(
            ledc_update_duty(
                PWM_MODE,
                PWM_CHANNEL));

        // /* Aguarda 500 ms entre pulsos */
        // vTaskDelay(pdMS_TO_TICKS(500));

        /* Reinicia a base de tempo da próxima aquisição */
        tempo_inicio = esp_timer_get_time();
    }

    /* Atualiza o duty cycle */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            PWM_MODE,
            PWM_CHANNEL,
            0));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            PWM_MODE,
            PWM_CHANNEL));

    printf("tempo_us,sinal_ad,adc_ads,adc_gpio14,pressao,pressao_total\n");

    for (uint16_t k = 0; k < TOTAL_AMOSTRAS; k++)
    {
        printf("%" PRIu32 ",%d,%d,%d,%.3f,%.3f\n",
               amostras[k].tempo_us,
               amostras[k].sinal_ad,
               amostras[k].adc_ads,
               amostras[k].adc_gpio14,
               amostras[k].pressao,
               amostras[k].pressao_total);
    }

    // ESP_ERROR_CHECK(
    //     gptimer_stop(gptimer));
}
