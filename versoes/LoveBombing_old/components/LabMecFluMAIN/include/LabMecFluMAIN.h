#ifndef LABMECFLUMAIN_H
#define LABMECFLUMAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ads111x.h"
#include "LabMecFluUART.h"
#include "LabMecFluPWM.h"

/**
 * Biblioteca para realizar processos
 * referentes ao laboratório de mecânica
 * dos fluidos, como calculo de pressao
 * pelo ADS1115.
 */

esp_err_t LabMecFlu_init(void);

#endif