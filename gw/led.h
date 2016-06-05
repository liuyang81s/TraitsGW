#ifndef TRAITS_LED_H
#define TRAITS_LED_H

TRAITScode led_init();
void led_close();
inline void led_on();
inline void led_off();
void led_flash(int period_ms, int count);
TRAITScode led_infinitly(int period_ms);
inline void led_report_success();
inline void led_error();

#endif

