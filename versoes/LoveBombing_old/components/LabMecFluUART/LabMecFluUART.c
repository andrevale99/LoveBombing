#include "LabMecFluUART.h"

static QueueHandle_t *handleUART_to_PWM;

static const char *TAG = "[LabMecFluUART]";

static esp_err_t ProcessBuffer(LabMecFluUART_Command_t *command,
                          uint8_t *buffer, uint8_t length)
{
    if (length < 2)
    {
        ESP_LOGW(TAG, "Buffer muito curto para processar");
        return ESP_ERR_INVALID_SIZE;
    }

    command->cmd = buffer[0];

    if (command->cmd == 'S' || command->cmd == 'T')
        command->value = atoi((const char *)&buffer[1]);
    else
    {
        ESP_LOGW(TAG, "Comando desconhecido: %c", command->cmd);
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

esp_err_t LabMecFluUART_init(QueueHandle_t *handleQueue)
{
    esp_err_t ret;

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ret = uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao instalar o driver UART: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ret = uart_param_config(UART_NUM_0, &uart_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao configurar os parâmetros UART: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ret = uart_set_pin(UART_NUM_0, TX, RX, -1, -1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao configurar os pinos UART: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "UART inicializada com sucesso");

    // xTaskCreate(vTaskUART, "vTaskUART", 2048, NULL, 2, NULL);
    xTaskCreatePinnedToCore(vTaskUART, "vTaskUART", 2048, NULL, 2, NULL, 0);

    handleUART_to_PWM = handleQueue;

    return ret;
}

void vTaskUART(void *pvParameters)
{
    char cdataRecv;
    LabMecFluUART_Command_t command;
    uint8_t buffer[BUF_SIZE];
    uint8_t cont = 0;
    while (1)
    {
        int len = uart_read_bytes(UART_NUM_0, &cdataRecv, 1,
                                  100 / portTICK_PERIOD_MS);
        if (len > 0)
        {

            buffer[cont++] = cdataRecv;
            uart_write_bytes(UART_NUM_0, &cdataRecv,
                             len);

            if (buffer[cont - 1] == '\r' || buffer[cont - 1] == '\n' || cont == BUF_SIZE)
            {
                if (ProcessBuffer(&command, buffer, cont) == ESP_OK)
                {
                    ESP_LOGI(TAG, "Comando recebido: %c, Valor: %d",
                             command.cmd, command.value);

                    xQueueSend(*handleUART_to_PWM, (void *)&command,
                               pdMS_TO_TICKS(10));
                }

                memset(buffer, 0, cont);
                cont = 0;
            }
        }
    }
}