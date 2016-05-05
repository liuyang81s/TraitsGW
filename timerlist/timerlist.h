#ifndef TIMER_LIST_H
#define TIMER_LIST_H

#include <string>
#include <list>
#include <stdint.h>
#include <event.h>

using namespace std;

typedef void (*TIMERFUNC)(void *arg);

class Timer
{
public:
	Timer();
	virtual ~Timer();

	bool set_time(timeval tv);
	bool set_time(string tv);
	timeval get_time() const;
	
	uint32_t get_period() const;
	void set_period(uint32_t period);

	TIMERFUNC onTime;
protected:
	timeval _tv;
	uint32_t _period;
};

class TimerList
{
public:
	TimerList();
	virtual ~TimerList();
	
	void init();
	void start();	

	void add_timer(Timer* tm);
	void delete_timer(Timer* tm);
	void update_timer(timeval tv);

	struct event* get_event();
	list<Timer*>* get_timers();		

protected:
	struct event_base* _base;
	struct event* _evTime;
	list<Timer*> _list;
};

#endif

