#ifndef UART_TASK_H
#define UART_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gptimer.h"

#include "env.h"
#include "cmd.h"
#include "uart.h"

#include "esp_log.h"

#define CMD_BUFFER_SIZE 128
#define SEND_BUFFER_SIZE 256

static const char *TAG_UART = "uart_task";
TaskHandle_t handleTask_uart = NULL;

static gptimer_handle_t timer = NULL;

static uartlove_config_t uart =
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

    uart_init(&uart);

    uint8_t *rx_data = (uint8_t *)malloc(ENV_UART_BUFFER_SIZE);
    char cmd_buffer[CMD_BUFFER_SIZE];
    char send_buffer[SEND_BUFFER_SIZE];
    cmd_t cmd = {0};
    uint16_t index = 0;
    data_t data = {0};
    uint64_t tempo_atual = 0;

    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz -> 1 contagem = 1 us
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&config, &timer));
    ESP_ERROR_CHECK(gptimer_enable(timer));
    ESP_ERROR_CHECK(gptimer_start(timer));

    gptimer_get_raw_count(timer, &tempo_atual);

    while (1)
    {
        int len = uart_read_bytes(uart_get_port(),
                                  rx_data,
                                  sizeof(rx_data),
                                  pdMS_TO_TICKS(5));

        if (len > 0)
        {
            // Ecoa os caracteres recebidos
            // uart_write_bytes(uart_get_port(), (const char *)rx_data, len);

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
                    memset(&cmd_buffer, 0, sizeof(cmd_buffer));
                }
                else
                {
                    // Armazena o caractere, evitando overflow
                    if (index < (ENV_UART_BUFFER_SIZE - 1))
                    {
                        cmd_buffer[index++] = c;
                    }
                }
            }
        }

        if (xQueueReceive(queue_data_to_uart, &data, 0) == pdTRUE)
        {
            gptimer_get_raw_count(timer, &tempo_atual);

            int tx_len = snprintf(send_buffer,
                                  SEND_BUFFER_SIZE,
                                  "%lld;%.2f;%.2f;%.3f;%i;%.2f\n",
                                  tempo_atual,
                                  data.pressaoTotal,
                                  data.pressao,
                                  data.vazao,
                                  data.duty,
                                  (((float)data.duty / 256.f * 100.f)));

            if (tx_len > 0)
            {

                uart_write_bytes(uart_get_port(), send_buffer, tx_len);
            }
        }
    }
}

TaskHandle_t *task_uart_get_handleTask(void)
{
    return &handleTask_uart;
}

#endif