#ifndef BOMBA_H
#define BOMBA_H

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

#define BOMBA_TIMER_DEFAULT 1

/**
 * @brief Estrutura de configuração do módulo heartbeat.
 */
typedef struct
{
    /**
     * @brief GPIOs associados à bomba 1.
     */
    int gpioBomb_1;

    /**
     * @brief GPIOs associados à bomba 2.
     */
    int gpioBomb_2;

} bomba_t;


/**
 * @brief Inicializa o módulo heartbeat e configura os canais PWM das bombas.
 *
 * Configura o temporizador do periférico LEDC para geração de sinais PWM
 * em 10 kHz e inicializa os canais associados às bombas 1 e 2 conforme os
 * GPIOs definidos na estrutura de configuração fornecida.
 *
 * A função verifica a validade dos pinos especificados e retorna
 * ESP_ERR_INVALID_ARG caso algum dos GPIOs seja inválido. Em caso de sucesso,
 * os canais PWM são configurados com duty cycle inicial igual a zero.
 *
 * @param[in] bomba Ponteiro para a estrutura de configuração do módulo
 *                  heartbeat, contendo os GPIOs associados às bombas.
 *
 * @return ESP_OK se a inicialização for concluída com sucesso.
 * @return ESP_ERR_INVALID_ARG se algum dos GPIOs especificados for inválido.
 *
 * @note O temporizador LEDC é configurado em modo de baixa velocidade
 *       (LEDC_LOW_SPEED_MODE) com resolução de 8 bits e frequência de 10 kHz.
 *
 * @warning O ponteiro @p heart deve apontar para uma estrutura válida e
 *          devidamente inicializada antes da chamada desta função.
 */
esp_err_t bomba_init(const bomba_t *);

/**
 * @brief Desinicializa o módulo heartbeat e libera os recursos utilizados.
 *
 * Interrompe a geração PWM dos canais associados às bombas 1 e 2,
 * desabilita os respectivos canais do periférico LEDC e restaura os
 * GPIOs utilizados para o estado padrão por meio de @c gpio_reset_pin().
 *
 * A desativação dos canais é realizada somente quando os GPIOs
 * correspondentes possuem valores válidos.
 *
 * @param[in] heart Ponteiro para a estrutura de configuração do módulo
 *                  heartbeat contendo os GPIOs das bombas.
 *
 * @return ESP_OK se a desinicialização for concluída com sucesso.
 * @return ESP_ERR_INVALID_ARG se o ponteiro @p heart for nulo.
 *
 * @note Os canais LEDC utilizados são:
 *       - LEDC_CHANNEL_0 para a bomba 1;
 *       - LEDC_CHANNEL_1 para a bomba 2.
 *
 * @warning O ponteiro @p heart deve apontar para uma estrutura válida
 *          previamente utilizada na inicialização do módulo.
 */
esp_err_t bomba_deinit(const bomba_t *);

#endif