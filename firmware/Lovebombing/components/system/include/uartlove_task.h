#ifndef uartlove_task_h
#define uartlove_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "env.h"
#include "letter.h"
#include "uartlove.h"

#include "esp_log.h"

static const char *TAG_UARTLOVE = "uartlove_task";
TaskHandle_t handle_uartlove = NULL;

static uartlove_config_t config =
    {
        .uartconfig = {
            .baud_rate = ENV_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        },
        .uartport = ENV_UART_PORT,
        .rxpin = ENV_UART_RX_GPIO,
        .txpin = ENV_UART_TX_GPIO,
        .buffersize = ENV_UART_BUFFER_SIZE,
};

// ====================================================+
// TASK
// ====================================================+
void task_uartlove(void *pvargs)
{
    ESP_LOGI(TAG_UARTLOVE, "uartlove task started");

    uartlove_init(&config);

    uint8_t *data = (uint8_t *)malloc(ENV_UART_BUFFER_SIZE);
    letter_t letter = {0};

    while (1)
    {
        int len = uart_read_bytes(uartlove_get_port(), data, (ENV_UART_BUFFER_SIZE - 1),
                                  950 / portTICK_PERIOD_MS);

        uart_write_bytes(uartlove_get_port(), (const char *)data, len);
        if (len > 0)
        {
            data[len] = '\0';
            letter_decoder(&letter, (char *)data);

            xQueueSend(queue_uartlove_to_mailman, &letter, -1);
        }
    }
}

TaskHandle_t *task_uartlove_get_handle(void)
{
    return &handle_uartlove;
}

#endif