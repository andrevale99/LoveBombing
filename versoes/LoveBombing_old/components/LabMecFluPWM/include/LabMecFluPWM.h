#ifndef LABMECFLUPWM_H
#define LABMECFLUPWM_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_log.h"

#include "LabMecFluUART.h"

#define BOMBA_GPIO GPIO_NUM_13

#define POTENCIA_MAX_BOMBA 95

/// @brief Funcao de configuracoes iniciais do periferico
/// do PWM
/// @param handleQueue fila que sera passada entre as tasks UART e PWM
/// @return ESP_OK Se o periferico for corretamente configurado
esp_err_t LabMecFluPWM_config(QueueHandle_t *handleQueue);

void vTaskPWM(void *pvParameters);

#endif
