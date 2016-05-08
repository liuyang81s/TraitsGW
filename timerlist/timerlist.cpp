#include <iostream>
#include <string>
#include <string.h>
#include <stdint.h>
#include <event.h>

#include "timer.h"
#include "timerlist.h"

using namespace std;

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
	
	list<Timer*>::iterator it; 	
    for(it = _list.begin(); it != _list.end(); ++it)
    {   
        delete (*it);
    } 	
}


//todo: add checking for if the time is correct
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

		tmlist->update_timer(tm->get_time());
	}	


	//如果定时器是周期性的，再次加入链表
	if((tm != NULL) & (tm->get_period() > 0))
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

static bool compare_timer (const Timer* first, const Timer* second)
{
    timeval t1 = first->get_time();
    timeval t2 = second->get_time();
    if(t1.tv_sec < t2.tv_sec)
        return true;
    else if(t1.tv_sec == t2.tv_sec) {
        if(t1.tv_usec <= t2.tv_usec)
            return true;
        else
            return false;   
    } else
        return false;   
}

void TimerList::add_timer(Timer* tm)
{
	_list.push_back(tm);
	_list.sort(compare_timer);
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


void TimerList::update_timer(timeval tv)
{
    list<Timer*>::iterator it; 

    for (it = _list.begin(); it != _list.end(); ++it) {
        timeval temp_tv = (*it)->get_time();
		temp_tv.tv_sec -= tv.tv_sec;
		if(temp_tv.tv_sec < 0)
			temp_tv.tv_sec = 0;
		//ignore usec
		(*it)->set_time(temp_tv);
    }  	
}


