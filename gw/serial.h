#ifndef TRAITS_SERIAL_H
#define TRAITS_SERIAL_H

typedef enum{
    UART_POLL = 0,
    UART_LISTEN,
    UART_INVALID,
}UART_MODE;

void serial_onTime(void* arg);
void* serial_listen_run(void* arg);
void* serial_poll_run(void* arg);
void serial_cleanup();

extern bool SERIAL_RUNNING;

#endif

