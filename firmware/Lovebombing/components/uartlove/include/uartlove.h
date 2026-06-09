#ifndef uartlove_h
#define uartlove_h

#include "driver/uart.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

#define UARTLOVE_BAUDRATE_DEFAULT 115200
#define UARTLOVE_UART_PORT_DEFAULT UART_NUM_0

#ifdef CONFIG_IDF_TARGET_ESP32S3
#define UARTLOVE_RX_GPIO_DEFAULT GPIO_NUM_44
#define UARTLOVE_TX_GPIO_DEFAULT GPIO_NUM_43
#elif CONFIG_IDF_TARGET_ESP32
#define UARTLOVE_RX_GPIO_DEFAULT GPIO_NUM_3
#define UARTLOVE_TX_GPIO_DEFAULT GPIO_NUM_1
#endif

#define UARTLOVE_BUFFER_SIZE_DEFAULT 1024

typedef struct
{
    uart_config_t uartconfig;
    int uartport;
    int rxpin;
    int txpin;
    int buffersize;
}uartlove_config_t;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t uartlove_init(uartlove_config_t *);

esp_err_t uartlove_deinit(void);

int uartlove_get_port(void);

bool uartlove_installed(void);

#ifdef __cplusplus
}
#endif

#endif