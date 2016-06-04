#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "traits.h"
#include "traits_elog.h"

using namespace std;

#define msleep(x) usleep(x * 1000)

#define LED_PATH "/sys/devices/platform/leds-gpio/leds/dragino2:red:system/brightness"

static int led_fd = -1;

TRAITScode led_init()
{
	led_fd = open(LED_PATH, O_RDWR);

	if(-1 == led_fd) {
		log_e("%s open failed", LED_PATH);
		return TRAITSE_FILE_OPEN_FAILED;
	}
	
	return TRAITSE_OK;
}

void led_close()
{
	if(-1 != led_fd) {
		close(led_fd);
		led_fd = -1;
	}
}


inline void led_on()
{
	static unsigned char ON = '1';

	write(led_fd, &ON, 1);
}

inline void led_off()
{
	static unsigned char OFF = '0';

	write(led_fd, &OFF, 1);
}


void led_flash(int period_ms, int count)
{
	if(count <= 0 || period_ms <= 0)
		return;

	int i = 0;
	bool is_on = true;	
	
	while(i++ < (count * 2)) {
		if(true == is_on) {
			led_on();
			is_on = false;
		} else {
			led_off();
			is_on = true;
		}

		msleep(period_ms);
	}
}

#define REPORT_SUCC_PERIOD 500
#define REPORT_SUCC_COUNT 3

inline void led_report_success()
{
	led_flash(REPORT_SUCC_PERIOD, REPORT_SUCC_COUNT);
}

