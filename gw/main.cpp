#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "gw.h"
#include "serial.h"
#include "timer.h"
#include "timerlist.h"
#include "main.h"

using namespace std;

#define RINGBUFFER_SIZE 2048

UnlockRingBuffer *rbuffer = NULL;
pthread_mutex_t rb_mutex;
pthread_cond_t  rb_cond;


int main()
{
	pthread_t t_gw;
	pthread_t t_hb;

	TraitsGW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");

    //初始化定时器链表
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    Timer tm;
	tm.set_time(tv);
	tm.set_period(5);
    tm.onTime = serial_onTime; 
    
    TimerList tmlist;
    tmlist.init();
    tmlist.add_timer(&tm);

    //初始化串口数据接收缓存    
    rbuffer = new UnlockRingBuffer(RINGBUFFER_SIZE);
    if(!rbuffer->init()) {
        cout << "ringbuffer init failed, serial thread exit" << endl;
        //todo: log
        return 0;
    }  

    //初始化同步变量
	pthread_mutex_init(&rb_mutex, NULL);
	pthread_cond_init(&rb_cond, NULL);

	int rc1 = pthread_create(&t_gw, NULL, gw_run, &gw);
	if(rc1){
		cout << "ERR: pthread_create failed with " << rc1 << endl;
		return -rc1;
	}

	int rc2 = pthread_create(&t_hb, NULL, hb_run, &gw);
	if(rc2){
		cout << "ERR: pthread_create failed with " << rc2 << endl;
		return -rc2;
	}
	

	HB_RUNNING = false;
	GW_RUNNING = false;

	pthread_join(t_hb, NULL);
	pthread_join(t_gw, NULL);
	
	pthread_mutex_destroy(&rb_mutex);
	pthread_cond_destroy(&rb_cond);

	if(rbuffer != NULL)
		delete rbuffer;

	cout << "main thread exit" << endl;

	return 0;
}

