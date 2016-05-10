#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "gw.h"
#include "serial.h"
#include "timer.h"
#include "timerlist.h"
#include "main.h"

using namespace std;

#define SERVER_URL "http://traits.imwork.net:10498/AnalyzeServer/system/"
#define RINGBUFFER_SIZE 2048

UnlockRingBuffer *rbuffer = NULL;
pthread_mutex_t rb_mutex;
pthread_cond_t  rb_cond;


int main()
{
	TraitsGW gw(SERVER_URL);
	
	while(true) {
		int init_ret = gw.request_init();
		if(TRAITSE_OK == init_ret)
			break;
		else if(TRAITSE_CONFIG_FILE_NOT_FOUND == init_ret ||
				TRAITSE_CONFIG_PARAM_NOT_FOUND == init_ret ||
				TRAITSE_MAC_NOT_FOUND == init_ret ||
				TRAITSE_MEM_ALLOC_FAILED == init_ret) {
			goto FATAL_OUT;
		} else {
			sleep(3);
		}
	}

	//初始化串口数据接收缓存    
    rbuffer = new UnlockRingBuffer(RINGBUFFER_SIZE);
    if(!rbuffer || !rbuffer->init()) {
        cout << "ringbuffer init failed" << endl;
        //todo: log
        goto RBUFFER_ERROR;
    }  

    //初始化同步变量
	pthread_mutex_init(&rb_mutex, NULL);
	pthread_cond_init(&rb_cond, NULL);
	
	pthread_t t_gw;
	pthread_t t_hb;
    pthread_t t_serial;
	
	int rc;
	rc = pthread_create(&t_hb, NULL, hb_run, &gw);
	if(rc){
		cout << "ERR: pthread_create failed with " << rc << endl;
		goto THREAD_HB_ERROR;
	}
	
    rc = pthread_create(&t_gw, NULL, gw_run, &gw);
	if(rc){
		cout << "ERR: pthread_create failed with " << rc << endl;
		goto THREAD_GW_ERROR;
	}

    //start serial thread, according uart mode
    //and ignore sendContent temporarily
    if(UART_POLL == gw.get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_poll_run, gw.get_timerlist());
    else if(UART_LISTEN == gw.get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_listen_run, NULL);
    else
        rc = -1;
    if(rc){
        cout << "ERR: pthread_create failed with " << rc << endl;
        goto FATAL_OUT;
    }                                    
                               
    SERIAL_RUNNING = false;     
    pthread_join(t_serial, NULL);


	pthread_join(t_gw, NULL);
	GW_RUNNING = false;

THREAD_GW_ERROR:
	HB_RUNNING = false;
	pthread_join(t_hb, NULL);

THREAD_HB_ERROR:
	pthread_mutex_destroy(&rb_mutex);
	pthread_cond_destroy(&rb_cond);

RBUFFER_ERROR:
	if(!rbuffer)
		delete rbuffer;
	
FATAL_OUT:
	while(true) {
		cout << "fatal error" << endl;	
		//TODO:LED indication
	}

	cout << "main thread exit" << endl;

	return 0;
}

