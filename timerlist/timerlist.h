#ifndef TIMER_LIST_H
#define TIMER_LIST_H

#include <string>
#include <list>
#include <stdint.h>
#include <event.h>
#include <pthread.h>

#include "timer.h"

class TimerList
{
public:
	TimerList();
	virtual ~TimerList();
	
	bool init();
	void start();	

	void add_timer(Timer* tm);
	void delete_timer(Timer* tm);
	void update_timer(const timeval& tv);
    void clean_timers();    

	struct event* get_event();
    std::list<Timer*>* get_timers();		

    int size();

	void lock();
	void unlock();

protected:
	struct event_base* _base;
	struct event* _evTime;
    std::list<Timer*> _list;
	
	pthread_mutex_t  _mutex;
};

#endif

