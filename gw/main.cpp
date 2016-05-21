#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

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

    log_i("GW exit...");

    close_elog();   

    exit(EXIT_SUCCESS);
}


int main()
{
	signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);                
    signal(SIGPIPE, SIG_IGN);                        
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN); 
    signal(SIGTERM, sigterm_handler);

	if(ELOG_NO_ERR != init_elog()) {
		cout << "elog init failed" << endl;
		goto FATAL_OUT;
	}

	log_i("GW starting...");

	gw = new TraitsGW(SERVER_URL);
	if(NULL == gw) {
		log_e("TraitsGW allocation failed");
		goto FATAL_OUT;
	}	
	if(TRAITSE_OK != gw->init()) {
		log_e(" TraitsGW init failed");
		goto GW_ERROR;
	}

	while(false) {
		int init_ret = gw->request_init();
		if(TRAITSE_OK == init_ret)
			break;
		else if(TRAITSE_CONFIG_FILE_NOT_FOUND == init_ret ||
				TRAITSE_CONFIG_PARAM_NOT_FOUND == init_ret ||
				TRAITSE_MAC_NOT_FOUND == init_ret ||
				TRAITSE_MEM_ALLOC_FAILED == init_ret) {
			goto GW_ERROR;
		} else {
			//todo: led indication
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

    //start serial thread, according uart mode	
    if(UART_POLL == gw->get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_poll_run, gw->get_timerlist());
    else if(UART_LISTEN == gw->get_uart_mode())
        rc = pthread_create(&t_serial, NULL, serial_listen_run, NULL);
    else {
		log_e("invalid uart_mode %d", (int)(gw->get_uart_mode()));
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
	if(NULL != rbuffer)
		delete rbuffer;

GW_ERROR:
	if(NULL != gw)
		delete gw;
	
FATAL_OUT:
    serial_cleanup(); //todo:where to put it?
	while(true) {
		//TODO:LED indication
        sleep(5);
	}

	log_i("GW exit...");

	close_elog();	

	return 0;
}

//todo:上述线程任何一个没有创建成功
//整个程序都没法正常运行，都应该给出错误提示，等候重启
//那么在重启之前,应该释放资源,结束掉其他线程
//每个线程分为异常结束和正常结束两种情况，
//可以通过pthread_join的参数返回值得知
//异常结束时，需要接着cancel掉其他的线程，以及释放资源


