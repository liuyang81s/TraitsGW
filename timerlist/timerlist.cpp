#include <iostream>
#include <string>
#include <stdint.h>
#include <event.h>

#include "timerlist.h"

using namespace std;


/*------------------------------------------------------
 * Timer definitions
 */
Timer::Timer()
{

}

Timer::Timer(timeval tv)
{
	_tv = tv;
	_period = 0;
}

Timer::Timer(string tv)
{
	//todo: parse the time sting to timeval
	_tv.tv_sec = 1;
	_tv.tv_usec = 0;
	_period = 0;
}

Timer::Timer(timeval tv, uint32_t period)
{
	_tv = tv;
	_period = period;
}

Timer::Timer(string tv, uint32_t period)
{
	//todo: parse the time sting to timeval
	_tv.tv_sec = 1;
	_tv.tv_usec = 0;
	_period = period;
}

Timer::~Timer()
{

}

void Timer::set_time(timeval tv)
{
	_tv = tv;
}

void Timer::set_time(string tv)
{
	//todo: parse the time sting to timeval
	 _tv.tv_sec = 1;
	_tv.tv_usec = 0;
}

timeval Timer::get_time()
{
	return _tv;
}

void Timer::set_period(uint32_t period)
{
	_period = period;
}

uint32_t Timer::get_period()
{
	return _period;
}


/*---------------------------------------------------------------
 * TimerList definitions
 */
TimerList::TimerList()
{
	_base = event_base_new();
}

TimerList::~TimerList()
{
	evtimer_del(_evTime);
	event_base_free(_base);
}

static void internal_onTimer(int sock, short event, void *arg)
{
	cout << "internal onTimer" << endl;

	TimerList* tmlist = (TimerList*)arg;
	struct event* ev = tmlist->get_event();	
	list<Timer*>* timers = tmlist->get_timers();

	//取队列头第一个timer,执行,删除
	Timer* tm = NULL;
	if(!timers->empty())
	{
		tm = timers->front();
		tm->onTime(arg);
		timers->pop_front();
	}	

	//如果定时器是周期性的，再次加入链表
	if((tm->get_period()) > 0)
	{
		timeval tv = tm->get_time();
		tv.tv_sec = tm->get_period();
		tm->set_time(tv);
	 	timers->push_back(tm);	
	}

	//若队列不空个,用下一个timer更新定时器
	if(!timers->empty())
	{
		Timer* tm = timers->front();
		timeval tv = (tm->get_time());
		evtimer_add(ev, &tv);
	}
}

void TimerList::init()
{
	_evTime = evtimer_new(_base, internal_onTimer, this);
}

void TimerList::start()
{
	if(_list.empty())
		return;
	else {
		Timer* tm = _list.front();
		timeval tv = tm->get_time();
	
		evtimer_add(_evTime, &tv);  
		event_base_dispatch(_base);
	}
}

void TimerList::add_timer(Timer* tm)
{
	_list.push_back(tm);
}

void TimerList::delete_timer(Timer* tm)
{
	//todo:
}

struct event* TimerList::get_event()
{
	return _evTime;
}

list<Timer*>* TimerList::get_timers()
{
	return &_list;
}


