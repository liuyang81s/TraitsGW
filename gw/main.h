#ifndef TRAITS_MAIN_H
#define TRAITS_MAIN_H

#include "unlock_ringbuffer.h"

extern UnlockRingBuffer *rbuffer;
extern pthread_mutex_t rb_mutex;
extern pthread_cond_t  rb_cond;

#endif
