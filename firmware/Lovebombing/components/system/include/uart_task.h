#ifndef UART_TASK_H
#define UART_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "env.h"
#include "cmd.h"
#include "uart.h"

#include "esp_log.h"

#define CMD_BUFFER_SIZE 128

static const char *TAG_UART = "uart_task";
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

    uint8_t *rx_data = (uint8_t *)malloc(ENV_UART_BUFFER_SIZE);
    char cmd_buffer[CMD_BUFFER_SIZE];
    cmd_t cmd = {0};
    uint16_t index = 0;

    while (1)
    {
        int len = uart_read_bytes(uart_get_port(),
                                  rx_data,
                                  sizeof(rx_data),
                                  10 / portTICK_PERIOD_MS);

        if (len > 0)
        {
            // Ecoa os caracteres recebidos
            uart_write_bytes(uart_get_port(), (const char *)rx_data, len);

            for (int i = 0; i < len; i++)
            {
                char c = (char)rx_data[i];

                // Enter pressionado?
                if (c == '\r' || c == '\n')
                {
                    // Ignora Enter se o buffer estiver vazio
                    if (index == 0)
                        continue;

                    // Finaliza a string sem incluir o Enter
                    cmd_buffer[index] = '\0';

                    cmd_decoder(&cmd, cmd_buffer);
                    xQueueSend(queue_uart_to_middleware, &cmd, portMAX_DELAY);

                    // Limpa o buffer para o próximo comando
                    index = 0;
                    memset(&cmd_buffer, 0, CMD_BUFFER_SIZE);
                }
                else
                {
                    // Armazena o caractere, evitando overflow
                    if (index < (CMD_BUFFER_SIZE - 1))
                    {
                        cmd_buffer[index++] = c;
                    }
                }
            }
        }
    }
}

TaskHandle_t *task_uart_get_handleTask(void)
{
    return &handleTask_uart;
}

#endif