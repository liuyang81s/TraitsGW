#include <iostream>
#include <string>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "traits.h"
#include "timer.h"

using namespace std;


/*------------------------------------------------------
 * Timer definitions
 */
Timer::Timer()
{
	_tv.tv_sec = 0;
	_tv.tv_usec = 0;
	_period = 0;
}

Timer::~Timer()
{
}

TRAITScode Timer::set_time(const timeval& tv)
{
	if(tv.tv_sec < 0 || tv.tv_usec < 0)
		return TRAITSE_TIME_FORMAT_ERROR;

	_tv = tv;

	return TRAITSE_OK;
}

//时间格式：2016-05-12 11:07:47
TRAITScode Timer::set_time(const string& tv)
{
    time_t cur_t;
    struct tm* cur_tm;
    struct tm dst_tm;

    time(&cur_t);
    cur_tm=localtime(&cur_t);

    memset(&dst_tm, 0, sizeof(dst_tm));
	if(NULL == strptime(tv.c_str(), "%Y-%m-%d %H:%M:%S", &dst_tm))
		return TRAITSE_TIME_FORMAT_ERROR;
    
	dst_tm.tm_mday = cur_tm->tm_mday;
   	dst_tm.tm_mon = cur_tm->tm_mon;
    dst_tm.tm_year = cur_tm->tm_year;
    dst_tm.tm_wday = cur_tm->tm_wday;
    dst_tm.tm_yday = cur_tm->tm_yday;
    dst_tm.tm_isdst = cur_tm->tm_isdst;

	int interval = mktime(&dst_tm) - cur_t;
	if(interval <= 0)
		return TRAITSE_TIME_EXPIRED; 

	_tv.tv_sec = interval;
	_tv.tv_usec = 0;
	_period = 0;

	return TRAITSE_OK;
}

timeval Timer::get_time() const
{
	return _tv;
}

void Timer::set_period(const uint32_t period)
{
	_period = period;
}

uint32_t Timer::get_period() const
{
	return _period;
}


/*------------------------------------------------------
 * WeeklyTimer definitions
 */
#define SEC_PER_DAY		(24 * 60 * 60)

WeeklyTimer::WeeklyTimer(uint8_t week_mask)
{
	//msb of 'week_mask' must be 0
	if(0x80 & week_mask)
		week_mask &= 0x7f;

	_period = SEC_PER_DAY;
	_week_mask = week_mask;
}

WeeklyTimer::~WeeklyTimer()
{
}

uint8_t WeeklyTimer::get_week_mask() const
{
	return _week_mask;
}

bool WeeklyTimer::set_week_mask(const uint8_t week_mask)
{
	//msb of 'week_mask' must be 0
	if(0x80 & week_mask)
		return false;
	
	_week_mask = week_mask;	

	return true;
}

uint32_t WeeklyTimer::get_period() const
{
	time_t cur_t;
	time(&cur_t);

	struct tm* cur_tm=localtime(&cur_t);
	int wday = cur_tm->tm_wday;

	//in libc style, wday of Sunday is 0
	//so we move the bit of Sunday to lsb
    uint8_t d = (0x40 & _week_mask) >> 6;
    uint8_t new_mask = ((_week_mask << 1) & 0x7f) | d;

	//find the next bit '1'
	int i;
    for(i = 1; i < 7; i++){ 
        if(new_mask & ((1 << (++wday) % 7))) 
            break;
    }

	return (i * SEC_PER_DAY); 
}

//时间格式：11:07:47
TRAITScode WeeklyTimer::set_time(const string& tv)
{
    time_t cur_t;
    struct tm* cur_tm;
    struct tm dst_tm;

    time(&cur_t);
    cur_tm=localtime(&cur_t);

    memset(&dst_tm, 0, sizeof(dst_tm));
	if(NULL == strptime(tv.c_str(), "%H:%M:%S", &dst_tm))
		return TRAITSE_TIME_FORMAT_ERROR;
    
	dst_tm.tm_mday = cur_tm->tm_mday;
   	dst_tm.tm_mon = cur_tm->tm_mon;
    dst_tm.tm_year = cur_tm->tm_year;
    dst_tm.tm_wday = cur_tm->tm_wday;
    dst_tm.tm_yday = cur_tm->tm_yday;
    dst_tm.tm_isdst = cur_tm->tm_isdst;
	int interval = mktime(&dst_tm) - cur_t;

	int wday = cur_tm->tm_wday;
	//in libc style, wday of Sunday is 0
	//so we move the bit of Sunday to lsb
    uint8_t sunday = (0x40 & _week_mask) >> 6;
    uint8_t new_mask = ((_week_mask << 1) & 0x7f) | sunday;
   
	 if(new_mask & (1 << wday)) {	//如果今天就是采集日,
		if(interval > 0){			//且采集时间还未到
			_tv.tv_sec = interval;	//设置定时器时间
		} else {					//若采集时间已过
			_tv.tv_sec = interval + get_period();	//定时器时间设置为下一个采集日的相同时刻
		}
	} else {	//今天不是采集日,定时器时间设置为最近采集日相同时刻
		_tv.tv_sec = interval + get_period();	
	}

	_tv.tv_usec = 0;
	_period = 0;

	return TRAITSE_OK;
}


