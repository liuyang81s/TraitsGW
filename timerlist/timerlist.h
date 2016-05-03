#ifndef TIMER_LIST_H
#define TIMER_LIST_H

#include <string>
#include <list>
#include <stdint.h>
#include <event.h>

using namespace std;

typedef void (*TIMERFUNC)(int sock, short event, void *arg);

class Timer
{
public:
	Timer();
	Timer(timeval tv);
	Timer(timeval tv, uint32_t period);
	Timer(string tv);
	Timer(string tv, uint32_t period);

	virtual ~Timer();

	void set_time(timeval tv);
	void set_time(string tv);
	timeval get_time();
	
	uint32_t get_period();
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

	struct event* get_event();
	list<Timer*>* get_timers();		

protected:
	struct event _evTime;
	list<Timer*> _list;
};

#endif

