#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "traits.h"

using namespace std;

#define msleep(x) usleep(x * 1000)

#define LED_PATH "/sys/devices/platform/leds-gpio/leds/dragino2:red:system/"
#define TRIGGER_PATH "trigger"
#define DELAY_ON_PATH "delay_on"
#define DELAY_OFF_PATH "delay_off" 

static int trigger_fd = -1;
static int delay_on_fd = -1;
static int delay_off_fd = -1;

static char trigger_none[] = "none";
static char trigger_timer[] = "timer";
static char trigger_on[] = "default-on";

TRAITScode led_init()
{
	if(-1 != trigger_fd)
		return TRAITSE_OK;	

	string trigger_path = string(LED_PATH) + string(TRIGGER_PATH);
 
	trigger_fd = open(trigger_path.c_str(), O_RDWR);	
	if(-1 == trigger_fd) {
		return TRAITSE_FILE_OPEN_FAILED;
	}
	
	return TRAITSE_OK;
}

void led_close()
{
	if(-1 != trigger_fd) {
		close(trigger_fd);
		trigger_fd = -1;
	}
	
	if(-1 != delay_on_fd) {
		close(delay_on_fd);
		delay_on_fd = -1;
	}

	if(-1 != delay_off_fd) {
		close(delay_off_fd);
		delay_off_fd = -1;
	}
}


inline void led_on()
{
	assert(-1 != trigger_fd);

	write(trigger_fd, trigger_on, sizeof(trigger_on));
}

inline void led_off()
{
	assert(-1 != trigger_fd);

	write(trigger_fd, trigger_none, sizeof(trigger_none));
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

TRAITScode led_infinitly(int period_ms)
{
	assert(-1 != trigger_fd);

	if(period_ms <= 0)
		return TRAITSE_ARG_INVALID;

	write(trigger_fd, trigger_timer, sizeof(trigger_timer));

	if(-1 == delay_on_fd) {	
		string delay_on_path = string(LED_PATH) + string(DELAY_ON_PATH);
		delay_on_fd = open(delay_on_path.c_str(), O_RDWR);	
		if(-1 == delay_on_fd) {
			return TRAITSE_FILE_OPEN_FAILED;
		}
	}

	if(-1 == delay_off_fd) {
		string delay_off_path = string(LED_PATH) + string(DELAY_OFF_PATH);
		delay_off_fd = open(delay_off_path.c_str(), O_RDWR);	
		if(-1 == delay_off_fd) {
			return TRAITSE_FILE_OPEN_FAILED;
		}
	}

	char str[16];
	memset(str, 0, 16);
	sprintf(str, "%d", period_ms);

	write(delay_on_fd, str, sizeof(str));
	write(delay_off_fd, str, sizeof(str));

	return TRAITSE_OK;
}


void led_report_success()
{
#define REPORT_SUCC_PERIOD 500
#define REPORT_SUCC_COUNT 3
	led_flash(REPORT_SUCC_PERIOD, REPORT_SUCC_COUNT);
}

void led_error()
{
#define LED_ERROR_PERIOD 150
	led_infinitly(LED_ERROR_PERIOD);
}

void led_ok()
{
	led_off();
}

