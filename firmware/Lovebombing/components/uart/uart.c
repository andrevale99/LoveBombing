#include "uart.h"

static const char *TAG = "uart";

static int internaluartport = -1;

esp_err_t uart_init(uartlove_config_t *config)
{
    ESP_LOGI(TAG, "uart install...");
    if (!config)
    {
        ESP_LOGI(TAG, "default install...");

        internaluartport = UART_PORT_DEFAULT;

        uart_config_t defaultconfig = {
            .baud_rate = UART_BAUDRATE_DEFAULT,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };

        ESP_ERROR_CHECK(uart_driver_install(UART_PORT_DEFAULT,
                                            UART_BUFFER_SIZE_DEFAULT * 2,
                                            0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(UART_PORT_DEFAULT, &defaultconfig));
        ESP_ERROR_CHECK(uart_set_pin(UART_PORT_DEFAULT,
                                     UART_TX_GPIO_DEFAULT, UART_RX_GPIO_DEFAULT,
                                     -1, -1));
        ESP_LOGI("UART",
                 "porta=%d tx=%d rx=%d baud=%ld buffer=%u",
                 UART_PORT_DEFAULT,
                 UART_TX_GPIO_DEFAULT,
                 UART_RX_GPIO_DEFAULT,
                 UART_BAUDRATE_DEFAULT,
                 UART_BUFFER_SIZE_DEFAULT);

        ESP_LOGI(TAG, "default install finished.");
    }
    else
    {
        ESP_LOGI(TAG, "custom install...");

        internaluartport = config->uartport;

        ESP_ERROR_CHECK(uart_driver_install(config->uartport, config->buffersize * 2,
                                            0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(config->uartport, &config->uartconfig));
        ESP_ERROR_CHECK(uart_set_pin(config->uartport,
                                     config->txpin, config->rxpin,
                                     -1, -1));
        ESP_LOGI("UART",
                 "porta=%d tx=%d rx=%d baud=%ld buffer=%u",
                 config->uartport,
                 config->txpin,
                 config->rxpin,
                 config->uartconfig.baud_rate,
                 config->buffersize);

        ESP_LOGI(TAG, "custom install finished.");
    }

    return ESP_OK;
}

esp_err_t uart_deinit(void)
{
    return uart_driver_delete(uart_get_port());
}

int uart_get_port(void)
{
    return internaluartport;
}

bool uart_installed(void)
{
    return uart_is_driver_installed(uart_get_port());
}