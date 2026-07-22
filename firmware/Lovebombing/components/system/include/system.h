#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);

void start_uart_task(void);
void start_middleware_task(void);
void start_bomba_task(void);
void start_ads111x_task(void);

#ifdef __cplusplus
}
#endif

#endif