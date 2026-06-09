#ifndef uartlove_task_h
#define uartlove_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "env.h"

#include "uartlove.h"

#include "esp_log.h"

static const char *TAG = "uartlove_task";
TaskHandle_t handle_uartlove = NULL;

static void process_command(char *rx_buffer)
{
    char *cmd;
    char *value;

    /* Remove \r e \n */
    rx_buffer[strcspn(rx_buffer, "\r\n")] = '\0';

    cmd = strtok(rx_buffer, " ");
    value = strtok(NULL, " ");

    if ((cmd != NULL) && (value != NULL))
    {
        printf("Comando: %s\n", cmd);
        printf("Valor: %s\n", value);

        int number = atoi(value);
        printf("Valor inteiro: %d\n", number);
    }
    else
    {
        printf("Formato invalido\n");
    }
}

void task_uartlove(void *pvargs)
{
    ESP_LOGI(TAG, "uartlove task started");

    uartlove_config_t config =
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

    uartlove_init(&config);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(ENV_UART_BUFFER_SIZE);

    while (1)
    {
        // Read data from the UART
        int len = uart_read_bytes(uartlove_get_port(), data, (ENV_UART_BUFFER_SIZE - 1),
                                  950 / portTICK_PERIOD_MS);
        // Write data back to the UART
        uart_write_bytes(uartlove_get_port(), (const char *)data, len);
        if (len)
        {
            data[len] = '\0';
            // ESP_LOGI(TAG, "Recv str: %s", (char *)data);
            process_command((char *)data);
        }
    }
}

TaskHandle_t *task_uartlove_get_handle(void)
{
    return &handle_uartlove;
}

#endif