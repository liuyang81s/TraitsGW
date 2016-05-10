#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <stdint.h>

typedef void (*TIMERFUNC)(void *arg);

class Timer
{
public:
	Timer();
	virtual ~Timer();

	bool set_time(const struct timeval& tv);
	bool set_time(const std::string& tv);
	struct timeval get_time() const;
	
	virtual uint32_t get_period() const;
	virtual void set_period(const uint32_t period);

	TIMERFUNC onTime;
protected:
	struct timeval _tv;
	uint32_t _period;
};

class WeeklyTimer: public Timer
{
public:
	WeeklyTimer(const uint8_t week_mask);
	virtual ~WeeklyTimer();

	uint32_t get_period() const;
	
	/*
	 * set_period has no effect here
	 */
	void set_period(const uint32_t period) { }

	uint8_t get_week_mask() const;
	/*
	 *  week_mask format, for example:
	 *  00000001, monday
	 *  00011111, monday -> friday
	 */	
	bool set_week_mask(const uint8_t week_mask);

protected:
	uint8_t _week_mask;	
};

#endif

