#ifndef TRAITS_SERIAL_H
#define TRAITS_SERIAL_H

#define POLL_MODE

typedef enum{
    UART_POLL = 0,
    UART_LISTEN,
    UART_INVALID,
}UART_MODE;

void serial_onTime(void* arg);
void* serial_run(void* arg);

extern bool SERIAL_RUNNING;

#endif

