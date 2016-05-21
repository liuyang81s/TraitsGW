#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "traits.h"
#include "gw.h"
#include "serial.h"
#include "timer.h"
#include "timerlist.h"
#include "traits_elog.h"
#include "main.h"

using namespace std;

#define RINGBUFFER_SIZE 2048

UnlockRingBuffer *rbuffer = NULL;
pthread_mutex_t rb_mutex;
pthread_cond_t  rb_cond;


int main()
{
	if(ELOG_NO_ERR != init_elog()) {
		cout << "elog init failed" << endl;
		return 0;
	}

	log_i("GW starting...");

	TraitsGW gw(SERVER_URL);
	gw.init();
	
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
        log_e("ringbuffer init failed");
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
		log_e("pthread_create failed with %d", rc);
		
		goto THREAD_HB_ERROR;
	}
	
    rc = pthread_create(&t_gw, NULL, gw_run, &gw);
	if(rc){
		log_e("pthread_create failed with %d", rc);
		goto THREAD_GW_ERROR;
	}

    //start serial thread, according uart mode	
    if(UART_POLL == gw.get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_poll_run, gw.get_timerlist());
    else if(UART_LISTEN == gw.get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_listen_run, NULL);
    else {
		log_e("invalid uart_mode %d", (int)(gw.get_uart_mode()));
        goto THREAD_SERIAL_ERROR;
	}    
    if(rc){
        log_e("pthread_create failed with %d", rc);
        goto THREAD_SERIAL_ERROR;
    }                                    
                               
    pthread_join(t_serial, NULL);
    SERIAL_RUNNING = false;     

THREAD_SERIAL_ERROR:
    //todo: how to make thread cancel?
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
		//TODO:LED indication
        sleep(5);
	}

	close_elog();	

	log_i("GW exit...");

	return 0;
}

