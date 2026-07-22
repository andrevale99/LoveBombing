#ifndef UART_TASK_H
#define UART_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "env.h"
#include "cmd.h"
#include "uart.h"

#include "esp_log.h"

static const char *TAG_UART= "uart_task";
TaskHandle_t handleTask_uart = NULL;

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
void task_uart(void *pvargs)
{
    ESP_LOGI(TAG_UART, "uart task started");

    uart_init(&config);

    uint8_t *data = (uint8_t *)malloc(ENV_UART_BUFFER_SIZE);
    cmd_t cmd = {0};

    while (1)
    {
        int len = uart_read_bytes(uart_get_port(), data, (ENV_UART_BUFFER_SIZE - 1),
                                  950 / portTICK_PERIOD_MS);

        uart_write_bytes(uart_get_port(), (const char *)data, len);
        if (len > 0)
        {
            data[len] = '\0';
            cmd_decoder(&cmd, (char *)data);

            xQueueSend(queue_uart_to_middleware, &cmd, -1);
        }
    }
}

TaskHandle_t *task_uart_get_handleTask(void)
{
    return &handleTask_uart;
}

#endif