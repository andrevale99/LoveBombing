#ifndef LABMECFLUART_H
#define LABMECFLUART_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_log.h"

#define RX GPIO_NUM_3
#define TX GPIO_NUM_1
#define BUF_SIZE 128

/**
 * Biblioteca responsável por
 * receber e enviar dados via UART.
 * Utilizado para fazer processamento
 * de comandos e gravar os dados 
 * no terminal.
 */

typedef struct {
    char cmd;
    int value;
} LabMecFluUART_Command_t;

/// @brief Funcao de configuracoes iniciais do periferico
/// do UART
/// @param handleQueue fila que sera passada entre as tasks UART e PWM
/// @return ESP_OK Se o periferico for corretamente configurado
esp_err_t LabMecFluUART_init(QueueHandle_t *handleQueue);

void vTaskUART(void *pvParameters);

#endif