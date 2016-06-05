#ifndef TRAITS_LED_H
#define TRAITS_LED_H

#include "traits.h"

TRAITScode led_init();
void led_close();
void led_on();
void led_off();
void led_flash(int period_ms, int count);
TRAITScode led_infinitly(int period_ms);
void led_report_success();
void led_error();
void led_ok();

#endif

