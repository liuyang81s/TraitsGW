#ifndef TRAITS_MAIN_H
#define TRAITS_MAIN_H

#define TRAITS_INFO
#define TRAITS_LOG
#define TRAITS_DEBUG
#define TRAITS_DEBUG_GW
//#define TRAITS_DEBUG_HB
#define TRAITS_DEBUG_SERIAL


#include "unlock_ringbuffer.h"

extern UnlockRingBuffer *rbuffer;
extern pthread_mutex_t rb_mutex;
extern pthread_cond_t  rb_cond;

#endif

