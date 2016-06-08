#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "selector.h"
#include "serial.h"
#include "main.h"
#include "gw.h"
#include "defines.h"
#include "devs.h"
#include "led.h"
#include "timerlist.h"
#include "traits.h"
#include "traits_elog.h"


#define DEVBUF_SIZE 128

using namespace std;

static string port = "";
static uint8_t devbuf[DEVBUF_SIZE];
static int devfd = -1;
static Selector* selector = NULL;
static TRAITScode thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 
static Device* dev = NULL;
static uint8_t* cmd = NULL;
static int cmd_len = 0;

bool SERIAL_RUNNING = false;

void dev_log(const char* prefix, uint8_t *buf, int size)    
{
#ifdef TRAITS_DEBUG_SERIAL
	cout << prefix << ": ";
    cout.fill('0');
    for(int i = 0; i < size; i++)
		cout << setw(2) << hex << (uint32_t)buf[i] << ' ';
	cout << endl;
#endif
}

void serial_onTime(void *arg)
{
    static struct timeval read_timeout = {3, 0};

	led_ok();

	if(devfd == -1) {			
		devfd = open(port.c_str(), O_RDWR);        	        
		if ( devfd == -1 ) {				
			led_error();
			log_e("%s: Open failed: %s", port.c_str(), strerror(errno));			
			return;
		} else {                     				
			log_i("%s: reopen", port.c_str());                     				
			selector->set_fd(devfd, READ);                 			
		}        
	}         

	memset(devbuf, 0, DEVBUF_SIZE);
	
	//write cmd to dev
    if(false == dev->send_cmd(cmd, cmd_len, devfd)) {
		led_error();
		log_e("%s: command send failed", port.c_str());
		return;	
	}

	while(true) {
		int r = selector->select(&read_timeout);
    	if(-1 == r) {			
			led_error();
			log_e("select error: %s", strerror(errno));        
	    	return;
		} else if (0 == r) {    //time out
								//error or
								//completion for non-fixed length packet            
			//log_w("select read timeout");
        	break;
    	}
		
		int total = 0;
		int devbytes = 0;		
		if(selector->fd_isset(devfd, READ)) {   
			devbytes = read(devfd, devbuf, DEVBUF_SIZE);
			if(devbytes <= 0) {
				led_error();
				log_e("%s: closed", port.c_str());
				close(devfd);
				selector->fd_clr(devfd, READ);
				devfd = -1;
				break;
			} else {
				total += devbytes;
				pthread_mutex_lock(&rb_mutex);
			    rbuffer->put(devbuf, devbytes);
		        pthread_cond_signal(&rb_cond);
        	    pthread_mutex_unlock(&rb_mutex);                
				dev_log(port.c_str(), devbuf, devbytes);				
			}
		}
		
		if((dev->get_packet_size() > 0) &&
			(total >= dev->get_packet_size()))
			break;
	} 
} 

void* serial_poll_run(void* arg)
{
	log_i("serial poll thread running...");	
	
	thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 
   
#ifdef TRAITS_DEBUG_GW
    list<Timer*>::iterator it;                                                                        
#endif
	TraitsGW* gw = (TraitsGW*)arg;
    TimerList* tmlist = gw->get_timerlist();
	port = gw->get_port();

    devfd = open(port.c_str(), O_RDWR);
    if ( devfd == -1 ) { 
		led_error();
        log_e("%s: Open failed: %s", port.c_str(), strerror(errno));
        goto out;
    } 

	selector = new Selector();
    if(NULL == selector) {
		led_error();
        log_e("Selector alloc failed");
        goto cleanup;
    }
	selector->set_fd(devfd, READ);

	dev = new CommonDev();
	if(NULL == dev) {
		log_e("Device alloc failed");
		goto cleanup;
	}
	dev->set_packet_size(gw->get_recv_len());

	cmd = gw->get_dev_cmd();
	cmd_len = gw->get_dev_cmd_len();

#ifdef TRAITS_DEBUG_GW
    cout << "timerlist size = " << tmlist->size() << endl;
    list<Timer*>* timers;
    timers = tmlist->get_timers();
    for(it = timers->begin(); it != timers->end(); ++it)
    {
        cout << "time tv_sec = " << (*it)->get_time().tv_sec << endl;
    }
#endif

    tmlist->start(); 	//if no pending, just return
	
	thr_ret = TRAITSE_THREAD_EXIT_SUCCESS;

	delete dev;

cleanup:
	serial_cleanup();
out:
	log_i("serial poll thread exit...");

	return &thr_ret;
}


void* serial_listen_run(void* arg)
{
	log_i("serial listen thread running...");	

	thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 
	
	TraitsGW* gw = (TraitsGW*)arg;
	port = gw->get_port();

	devfd = open(port.c_str(), O_RDWR);
    if ( devfd == -1 ) {
	   led_error();
       log_e("%s: Open failed: %s", port.c_str(), strerror(errno)); 
       goto out;
    } 

    selector = new Selector();
    if(NULL == selector) {                   
		led_error();
        log_e("Selector alloc failed");
        goto cleanup;
    }   
    selector->set_fd(devfd, READ);
    
	memset(devbuf, 0, DEVBUF_SIZE);

	SERIAL_RUNNING = true;	
	while(SERIAL_RUNNING) {
		if(selector->select(NULL) == -1) {
			log_e("select error: %s", strerror(errno));
			sleep(2);
            continue;
		} 
	
		int devbytes = 0;
		if(selector->fd_isset(devfd, READ)) {   
			devbytes = read(devfd, devbuf, DEVBUF_SIZE);
			if(devbytes <= 0) {
				log_e("%s: closed", port.c_str());
				close(devfd);
				selector->fd_clr(devfd, READ);
				devfd = -1;
			
				sleep(5);	//wait for reopen dev

				devfd = open(port.c_str(), O_RDWR);
			    if ( devfd == -1 ) {
					led_error();
					log_e("%s: Open failed: %s", port.c_str(), strerror(errno));
			        goto cleanup;
    			} else {
					log_i("%s: reopen", port.c_str());
    				selector->set_fd(devfd, READ);
				}
				continue;
			} else {
				pthread_mutex_lock(&rb_mutex);
				rbuffer->put(devbuf, devbytes);
				pthread_cond_signal(&rb_cond);
				pthread_mutex_unlock(&rb_mutex);
				dev_log(port.c_str(), devbuf, devbytes);				
			}
        } 
	}

	thr_ret = TRAITSE_THREAD_EXIT_SUCCESS;

cleanup:
	serial_cleanup();
out:    
	log_i("serial listen thread exit...");
   
	return &thr_ret;
}

void serial_cleanup()
{
#if 1
    cout << "serial_cleanup" <<endl;
#endif

	if(NULL != selector) {
		delete selector;
		selector = NULL;
	}

    if(-1 != devfd) {
        close(devfd);
		devfd = -1;
	}
}

