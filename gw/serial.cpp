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
#include "defines.h"
#include "devs.h"
#include "timerlist.h"
#include "traits.h"
#include "traits_elog.h"


#define DEVBUF_SIZE 128

using namespace std;

#ifdef DEBUG_ON_PC
static const char DEVNAME[] = "/dev/ttyS0";
#else
//static const char DEVNAME[] = "/dev/tty232";
//static const char DEVNAME[] = "/dev/tty485";
//static const char DEVNAME[] = "/dev/ttyUSB1";
//static const char DEVNAME[] = "/dev/ttyUSB0";
#endif

static uint8_t devbuf[DEVBUF_SIZE];
static int devfd = -1;
static Selector* selector = NULL;
static TRAITScode thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 

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
    static struct timeval read_timeout = {5, 0};

    if(devfd == 0)
        return;

    cout << "onTime" << endl;

	//todo: should not make a new Device in timer handler
    Device* dev = new SONBEST_SD5110B(0x1);
    //todo: if dev=NULL

    dev->send_cmd(NULL, devfd);
    //todo: if fail

	//todo: find a way to remove this line	
	delete dev;


	memset(devbuf, 0, DEVBUF_SIZE);
	int r = selector->select(&read_timeout);
    if(-1 == r) {			
		log_e("select error: %s", strerror(errno));        
	    return;
	} else if (0 == r) {    //time out            
		log_w("select read timeout");
        return;
    }
		
	int devbytes = 0;		
	if(selector->fd_isset(devfd, READ)) {   
		devbytes = read(devfd, devbuf, DEVBUF_SIZE);
		if(devbytes <= 0) {
			cout << DEVNAME << " closed" << endl;
			//todo: log
			close(devfd);
			selector->fd_clr(devfd, READ);
			//todo: reopen dev			
		} else {
			pthread_mutex_lock(&rb_mutex);
		    rbuffer->put(devbuf, devbytes);
	        pthread_cond_signal(&rb_cond);
            pthread_mutex_unlock(&rb_mutex);                
			dev_log(DEVNAME, devbuf, devbytes);				
		}
	} 
} 

void* serial_poll_run(void* arg)
{
	log_i("serial poll thread running...");	
	
	thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 
   
#ifdef TRAITS_DEBUG_GW
    list<Timer*>::iterator it;                                                                        
#endif
    TimerList* tmlist = (TimerList*)arg;

    devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        log_e("%s: Open failed", DEVNAME);
        goto out;
    } 

	selector = new Selector();
    if(NULL == selector) {
        log_e("Selector alloc failed");
        goto close_dev;
    }
	selector->set_fd(devfd, READ);

#ifdef TRAITS_DEBUG_GW
    cout << "timerlist size = " << tmlist->size() << endl;
    list<Timer*>* timers;
    timers = tmlist->get_timers();
    for(it = timers->begin(); it != timers->end(); ++it)
    {
        cout << "time tv_sec = " << (*it)->get_time().tv_sec << endl;
    }
#endif

    tmlist->start(); 	//if no pending, it return
	
	thr_ret = TRAITSE_THREAD_EXIT_SUCCESS;

close_dev:
    close(devfd);
	devfd = -1;
out:
	log_i("serial poll thread exit...");

	return &thr_ret;
}


void* serial_listen_run(void* arg)
{
	log_i("serial listen thread running...");	

	thr_ret = TRAITSE_THREAD_EXIT_ABNORMAL; 

	devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) {
       log_e("%s: Open failed", DEVNAME); 
       goto out;
    } 

    selector = new Selector();
    if(NULL == selector) {                   
        log_e("Selector alloc failed");
        goto close_dev;
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
				cout << DEVNAME << " closed" << endl;
				//todo: log
				close(devfd);
				selector->fd_clr(devfd, READ);
				break;
				//todo: reopen dev
			} else {
				pthread_mutex_lock(&rb_mutex);
				rbuffer->put(devbuf, devbytes);
				pthread_cond_signal(&rb_cond);
				pthread_mutex_unlock(&rb_mutex);
				dev_log(DEVNAME, devbuf, devbytes);				
			}
        } 
	}

	thr_ret = TRAITSE_THREAD_EXIT_SUCCESS;

close_dev:
    close(devfd);
	devfd = -1;
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

