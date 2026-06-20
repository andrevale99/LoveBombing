#ifndef queue_task_h
#define queue_task_h

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "letter.h"

QueueHandle_t queue_uartlove_to_mailman = NULL;

void queue_start(void)
{
    queue_uartlove_to_mailman = xQueueCreate(3, sizeof(letter_t));
}

#endif