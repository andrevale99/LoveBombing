#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"

#define ADC_ATTENUATION ADC_ATTEN_DB_12
#define ADC_BIT_WIDTH ADC_BITWIDTH_12

#include "driver/ledc.h"
#include "driver/gptimer.h"
#include "esp_adc/adc_oneshot.h"

#include "esp_err.h"
#include "esp_log.h"

#include "ads111x.h"

#include "esp_timer.h"

#define PWM_GPIO GPIO_NUM_13
#define PWM_FREQ_HZ 5000

#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0
#define PWM_CHANNEL LEDC_CHANNEL_0

#define PWM_RESOLUTION LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255

#define LEN_SINAL_AD 201

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
    adc_init();

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

uint16_t indice = 0;

while (1)
{
    /* Atualiza o duty cycle do PWM */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            PWM_MODE,
            PWM_CHANNEL,
            sinal_AD[indice]));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            PWM_MODE,
            PWM_CHANNEL));

    /* Leitura do ADC do GPIO14 */
    int adc;

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc2_handle,
            ADC_CHANNEL_6,
            &adc));

    /* Conversão aproximada para tensão */
    float tensao = (3.3f * adc) / 4095.0f;

    printf("Posicao: %3u | PWM: %3d | ADC: %4d | Tensao: %.3f V\n",
           indice,
           sinal_AD[indice],
           adc,
           tensao);

    /* Próxima posição do vetor */
    indice++;

    if (indice >= LEN_SINAL_AD)
        indice = 0;
    

    /* Atualiza a cada 100 ms */
    vTaskDelay(pdMS_TO_TICKS(100));
}
}