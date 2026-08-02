#ifndef queue_task_h
#define queue_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "cmd.h"

static char *TAG_QUEUE_START = "queue_system";

QueueHandle_t queue_uart_to_middleware = NULL;
QueueHandle_t queue_middleware_to_bomba = NULL;
QueueHandle_t queue_data_to_uart = NULL;
QueueHandle_t queue_bomba_to_data = NULL;

void queue_start(void)
{
    queue_uart_to_middleware = xQueueCreate(3, sizeof(cmd_t));
    if (queue_uart_to_middleware == NULL)
        ESP_LOGE(TAG_QUEUE_START, "Erro ao inicializar a queue uart->middleware");
    ESP_LOGI(TAG_QUEUE_START, "Queue uart->middleware inicializada");

    queue_middleware_to_bomba = xQueueCreate(3, sizeof(cmd_t));
    if (queue_middleware_to_bomba == NULL)
        ESP_LOGE(TAG_QUEUE_START, "Erro ao inicializar a queue middleware->bomba");
    ESP_LOGI(TAG_QUEUE_START, "Queue middleware->bomba inicializada");

    queue_data_to_uart = xQueueCreate(3, sizeof(data_t));
    if (queue_data_to_uart == NULL)
        ESP_LOGE(TAG_QUEUE_START, "Erro ao inicializar a queue data->uart");
    ESP_LOGI(TAG_QUEUE_START, "Queue data->uart inicializada");

    queue_bomba_to_data = xQueueCreate(100, sizeof(int));
    if (queue_bomba_to_data == NULL)
        ESP_LOGE(TAG_QUEUE_START, "Erro ao inicializar a queue bomba->data");
    ESP_LOGI(TAG_QUEUE_START, "Queue dbomba->data inicializada");
}

#endif