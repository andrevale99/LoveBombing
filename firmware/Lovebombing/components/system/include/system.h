#ifndef system_h
#define system_h

#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);

void start_uartlove_task(void);
void start_mailman_task(void);

#ifdef __cplusplus
}
#endif

#endif