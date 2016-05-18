#ifndef TRAITS_ELOG_H
#define TRAITS_ELOG_H

#include <stdio.h>
#include "elog.h"

ElogErrCode init_elog(void);
ElogErrCode close_elog(void);

#define log_a(...) elog_a("TraitsGW", __VA_ARGS__)
#define log_e(...) elog_e("TraitsGW", __VA_ARGS__)
#define log_w(...) elog_w("TraitsGW", __VA_ARGS__)
#define log_i(...) elog_i("TraitsGW", __VA_ARGS__)
#define log_d(...) elog_d("TraitsGW", __VA_ARGS__)
#define log_v(...) elog_v("TraitsGW", __VA_ARGS__)

#endif

