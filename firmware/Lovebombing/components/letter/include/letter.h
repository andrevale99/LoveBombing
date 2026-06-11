#ifndef letter_h
#define letter_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LETTER_CMD_SIZE 16

typedef struct
{
    char cmd[LETTER_CMD_SIZE];
    int value;
}letter_t;


/**
 * @brief Decodifica uma string de comando e armazena os resultados em uma estrutura.
 *
 * A função recebe uma string no formato:
 *
 * @code
 * <comando> <valor>
 * @endcode
 *
 * Remove os caracteres de fim de linha ('\r' e '\n'), separa a string em
 * dois tokens utilizando espaço como delimitador e armazena o comando e o
 * valor na estrutura informada.
 *
 * Exemplo:
 *
 * @code
 * "SET 100\r\n"
 * @endcode
 *
 * Resulta em:
 *
 * @code
 * letter->cmd   = "SET"
 * letter->value = 100
 * @endcode
 *
 * @param[out] letter Ponteiro para a estrutura onde os dados decodificados
 *                    serão armazenados.
 * @param[in,out] rx_buffer Buffer contendo a string a ser decodificada.
 *                          O conteúdo do buffer é modificado pela função.
 *
 * @return 0  Decodificação realizada com sucesso.
 * @return -1 Ponteiro nulo fornecido em algum parâmetro.
 * @return -2 Formato inválido da mensagem ou parâmetros ausentes.
 *
 * @note A função utiliza strtok(), portanto o conteúdo original de
 *       @p rx_buffer é alterado.
 */
int8_t letter_decoder(letter_t *, char *);

#endif