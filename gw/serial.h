#ifndef TRAITS_SERIAL_H
#define TRAITS_SERIAL_H

#define POLL_MODE

typedef enum{
    LISTEN,
    POLL,
    INVALID,
}WORK_MODE;


void* serial_run(void* arg);

extern bool SERIAL_RUNNING;

#endif

