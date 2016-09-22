#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "traits.h"
#include "gw.h"
#include "serial.h"
#include "timer.h"
#include "timerlist.h"
#include "traits_elog.h"
#include "led.h"
#include "main.h"

using namespace std;

#define RINGBUFFER_SIZE 2048

UnlockRingBuffer *rbuffer = NULL;
pthread_mutex_t rb_mutex;
pthread_cond_t  rb_cond;
static TraitsGW* gw = NULL; 

static void sigterm_handler(int sig)
{
    cout << "sigterm handler" << endl;

    GW_RUNNING = false;
    HB_RUNNING = false;
        
    serial_cleanup();
    
	if(NULL != rbuffer)
        delete rbuffer;

    if(NULL != gw) 
        delete gw; 

	led_ok();
	led_close();

    log_i("GW exit...");
    close_elog();   

    exit(EXIT_SUCCESS);
}


int main()
{
	//do led init first
	if(TRAITSE_OK != led_init()) {
		exit(EXIT_FAILURE);
	} else {
		led_ok();
	}

	signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);                
    signal(SIGPIPE, SIG_IGN);                        
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN); 
    signal(SIGTERM, SIG_IGN);

	pid_t pid = fork();
	if(pid < 0) {
		cout << "fork error" << endl;
		goto FATAL_OUT;
	} else if(pid > 0) {
		exit(EXIT_SUCCESS);
	}

	setsid();

    char szPath[1024];  
    if(getcwd(szPath, sizeof(szPath)) == NULL)  
    {  
        cout << "getcwd failed" << endl;  
		goto FATAL_OUT;
    }  
    else  
    {  
        chdir(szPath);  
    }  
  
	umask(0);

    signal(SIGTERM, sigterm_handler);

	if(ELOG_NO_ERR != init_elog()) {
		cout << "elog init failed" << endl;
		goto FATAL_OUT;
	}

	log_i("GW starting...");

	gw = new TraitsGW();
	if(NULL == gw) {
		log_e("TraitsGW allocation failed");
		goto GW_ALLOC_ERROR;
	}	
	if(TRAITSE_OK != gw->init()) {
		log_e(" TraitsGW init failed");
		goto GW_ERROR;
	}

	while(true) {
		int init_ret = gw->request_init();
		if(TRAITSE_OK == init_ret)
			break;
		else if(TRAITSE_CONFIG_FILE_NOT_FOUND == init_ret ||
				TRAITSE_CONFIG_PARAM_NOT_FOUND == init_ret ||
				TRAITSE_MAC_NOT_FOUND == init_ret ||
				TRAITSE_MEM_ALLOC_FAILED == init_ret) {
			goto GW_ERROR;
		} else {
			led_error();
			sleep(3);
		}
	}

	//初始化串口数据接收缓存    
    rbuffer = new UnlockRingBuffer(RINGBUFFER_SIZE);
    if(NULL == rbuffer) {
		log_e("UnlockRingBuffer allocation failed");
		goto GW_ERROR;
	}
	if(false == rbuffer->init()) {
        log_e("UnlockRingBuffer init failed");
        goto RBUFFER_ERROR;
    }  

    //初始化同步变量
	pthread_mutex_init(&rb_mutex, NULL);
	pthread_cond_init(&rb_cond, NULL);
	
	pthread_t t_gw;
	pthread_t t_hb;
    pthread_t t_serial;
	TRAITScode thr_serial_ret;
	thr_serial_ret = TRAITSE_THREAD_EXIT_ABNORMAL;
	int rc;

	rc = pthread_create(&t_hb, NULL, hb_run, gw);
	if(rc){
		log_e("pthread_create failed with %d", rc);
		goto THREAD_HB_ERROR;
	}
	
    rc = pthread_create(&t_gw, NULL, gw_run, gw);
	if(rc){
		log_e("pthread_create failed with %d", rc);
		goto THREAD_GW_ERROR;
	}
#if 0
	//to check if t_gw thread exit abnormal
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 10;	//wait for 2s
	rc = pthread_timedjoin_np(t_gw, NULL, &ts);
	if(0 != rc) {
		cout << strerror(rc) << endl; 
		cout << "t_gw exit:" << strerror(errno) <<endl;
		goto THREAD_GW_ERROR;
	}
#endif

    //start serial thread, according uart mode	
    if(UART_POLL == gw->get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_poll_run, gw);
    else if(UART_LISTEN == gw->get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_listen_run, gw);
    else {
		log_e("invalid uart_mode %d", (int)(gw->get_uart_mode()));
        goto THREAD_SERIAL_ERROR;
	}    
    if(rc){
        log_e("pthread_create failed with %d", rc);
        goto THREAD_SERIAL_ERROR;
    }                                    

	void* res;
    pthread_join(t_serial, &res);
	thr_serial_ret = *((TRAITScode*)res); 
#if 1
	cout << "join ret = " << thr_serial_ret << endl;
#endif
    SERIAL_RUNNING = false;     

THREAD_SERIAL_ERROR:
	//1\t_serial exit abnormal 
	//2\t_serial create failed
	if(TRAITSE_THREAD_EXIT_ABNORMAL == thr_serial_ret) {
    	serial_cleanup();
		pthread_cancel(t_gw);
	}
	GW_RUNNING = false;
	pthread_join(t_gw, NULL);
#if 1
cout << "t_gw canceled" <<endl;
#endif

THREAD_GW_ERROR:
	HB_RUNNING = false;
	pthread_cancel(t_hb);
	pthread_join(t_hb, NULL);
#if 1
cout << "t_hb canceled" <<endl;
#endif

THREAD_HB_ERROR:
	pthread_mutex_destroy(&rb_mutex);
	pthread_cond_destroy(&rb_cond);

RBUFFER_ERROR:
	if(NULL != rbuffer){
		delete rbuffer;
		rbuffer = NULL;
	}

GW_ERROR:
	if(NULL != gw) {
		delete gw;
		gw = NULL;
	}
	
GW_ALLOC_ERROR:
	log_i("GW exit...");

	close_elog();	

FATAL_OUT:
	led_error();
	led_close();

	return 0;
}

