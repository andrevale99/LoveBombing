/*
 * Medição de largura de pulso em tempo real
 * usando:
 * - GPIO com interrupção
 * - GPTimer
 * - LEDC PWM
 * - Timer periódico da ESP-IDF
 *
 * Recursos:
 * - Captura largura do pulso em nível alto
 * - Resolução de 1 us
 * - PWM 8 bits no GPIO 13
 * - Duty varia automaticamente:
 *      sobe até 255
 *      depois desce até 0
 * - Passo de 25
 * - Atualização a cada 5 segundos
 *
 * ESP-IDF v5+
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/ledc.h"

#include "esp_log.h"
#include "esp_attr.h"
#include "esp_timer.h"

//
// GPIO captura
//
#define INPUT_GPIO GPIO_NUM_14

//
// PWM
//
#define PWM_GPIO           GPIO_NUM_13
#define PWM_FREQUENCY_HZ   5000

#define PWM_MODE           LEDC_LOW_SPEED_MODE
#define PWM_TIMER          LEDC_TIMER_0
#define PWM_CHANNEL        LEDC_CHANNEL_0

//
// PWM 8 bits
//
#define PWM_MAX_DUTY       255
#define PWM_STEP           25

static const char *TAG = "PWM_CAPTURE";

//
// GPTimer
//
static gptimer_handle_t gptimer = NULL;

//
// Captura do pulso
//
volatile uint64_t pulse_time_us = 0;

//
// PWM
//
volatile int32_t pwm_duty = 0;

//
// Direção:
// true  -> subindo
// false -> descendo
//
volatile bool pwm_up = true;

//
// Timer periódico
//
static esp_timer_handle_t periodic_timer;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    static bool measuring = false;

    int level = gpio_get_level(INPUT_GPIO);

    if (level)
    {
        //
        // Borda de subida
        //

        gptimer_set_raw_count(gptimer, 0);

        gptimer_start(gptimer);

        measuring = true;
    }
    else
    {
        //
        // Borda de descida
        //

        if (measuring)
        {
            gptimer_stop(gptimer);

            uint64_t count = 0;

            gptimer_get_raw_count(gptimer, &count);

            //
            // 1 tick = 1 us
            //

            pulse_time_us = count;

            measuring = false;
        }
    }
}

static void pwm_init(void)
{
    //
    // Timer LEDC
    //

    ledc_timer_config_t ledc_timer = {
        .speed_mode      = PWM_MODE,
        .timer_num       = PWM_TIMER,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz         = PWM_FREQUENCY_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&ledc_timer));

    //
    // Canal LEDC
    //

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = PWM_GPIO,
        .speed_mode = PWM_MODE,
        .channel    = PWM_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = PWM_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&ledc_channel));
}

static void duty_timer_callback(void *arg)
{
    //
    // Controle subida/descida
    //

    if (pwm_up)
    {
        pwm_duty += PWM_STEP;

        if (pwm_duty >= PWM_MAX_DUTY)
        {
            pwm_duty = PWM_MAX_DUTY;

            pwm_up = false;
        }
    }
    else
    {
        pwm_duty -= PWM_STEP;

        if (pwm_duty <= 0)
        {
            pwm_duty = 0;

            pwm_up = true;
        }
    }

    //
    // Atualiza PWM
    //

    ledc_set_duty(
        PWM_MODE,
        PWM_CHANNEL,
        pwm_duty);

    ledc_update_duty(
        PWM_MODE,
        PWM_CHANNEL);
}

void app_main(void)
{
    //
    // Configuração GPTimer
    //

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,

        //
        // 1 MHz
        // 1 tick = 1 us
        //

        .resolution_hz = 1000000,
    };

    ESP_ERROR_CHECK(
        gptimer_new_timer(
            &timer_config,
            &gptimer));

    ESP_ERROR_CHECK(
        gptimer_enable(gptimer));

    //
    // Configuração GPIO captura
    //

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << INPUT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&io_conf));

    //
    // ISR GPIO
    //

    ESP_ERROR_CHECK(
        gpio_install_isr_service(0));

    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            INPUT_GPIO,
            gpio_isr_handler,
            NULL));

    //
    // Inicializa PWM
    //

    pwm_init();

    //
    // Timer periódico
    // 5 segundos
    //

    esp_timer_create_args_t timer_args = {
        .callback = &duty_timer_callback,
        .name = "duty_timer"
    };

    ESP_ERROR_CHECK(
        esp_timer_create(
            &timer_args,
            &periodic_timer));

    ESP_ERROR_CHECK(
        esp_timer_start_periodic(
            periodic_timer,
            5000000/2)); // us

    ESP_LOGI(TAG, "Sistema iniciado");

    while (1)
    {
        //
        // Conversões
        //

        float pulse_ms =
            pulse_time_us / 1000.0f;

        float duty_percent =
            ((float)pwm_duty / PWM_MAX_DUTY) * 100.0f;

        //
        // Log em tempo real
        //

        ESP_LOGI(TAG,
                //  "Pulso: %llu us | %.3f ms | Duty: %ld | %.1f%%",
                "%llu\t%.3f\t%ld\t%.1f%%",
                 pulse_time_us,
                 pulse_ms,
                 pwm_duty,
                 duty_percent);

        //
        // Atualização rápida
        //

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}