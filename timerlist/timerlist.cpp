#include <iostream>
#include <event.h>
#include <pthread.h>

#include "timer.h"
#include "timerlist.h"
#include "traits_elog.h"

using namespace std;

//todo: add checking for if the time is correct
static void internal_onTimer(int sock, short event, void *arg)
{
#if 1 //debug
	cout << "internal onTimer" << endl;
#endif

	TimerList* tmlist = (TimerList*)arg;
	tmlist->lock();

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

	//若队列不空,用下一个timer更新定时器
	if(!timers->empty())
	{
		Timer* tm = timers->front();
		timeval tv = (tm->get_time());
		evtimer_add(ev, &tv);
	}

	tmlist->unlock();
}

/*---------------------------------------------------------------
 * TimerList definitions
 */
TimerList::TimerList()
{
	_base = NULL;
	_evTime = NULL;
	_list.clear();	
}

TimerList::~TimerList()
{
	evtimer_del(_evTime);
	event_base_free(_base);

    clean_timers();

	pthread_mutex_destroy(&_mutex);
}

bool TimerList::init()
{
	_base = event_base_new();
	if(NULL == _base) {
		log_e("create new event_base for libevent failed");
		return false;
	}
	_evTime = evtimer_new(_base, internal_onTimer, this);
	if(NULL == _evTime) {
		log_e("create new event for libevent failed");
		return false;
	}

	pthread_mutex_init(&_mutex, NULL);

	return true;
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
	pthread_mutex_lock(&_mutex);	

	_list.push_back(tm);
	_list.sort(compare_timer);

	pthread_mutex_unlock(&_mutex);	
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

int TimerList::size()
{
	return _list.size();
}

void TimerList::lock()
{
	pthread_mutex_lock(&_mutex);
}

void TimerList::unlock()
{
	pthread_mutex_unlock(&_mutex);
}

void TimerList::clean_timers()
{
	pthread_mutex_lock(&_mutex);	

	list<Timer*>::iterator it; 	
    for(it = _list.begin(); it != _list.end(); ++it)
    {   
        delete (*it);
    } 	
	_list.clear();

	pthread_mutex_unlock(&_mutex);
}

void TimerList::update_timer(const timeval& tv)
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

