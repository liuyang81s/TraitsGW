#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "selector.h"
#include "serial.h"
#include "main.h"
#include "devs.h"
#include "timerlist.h"


#define DEVBUF_SIZE 128

using namespace std;

static const char DEVNAME[] = "/dev/ttyS0";
//static const char DEVNAME[] = "/dev/tty232";
//static const char DEVNAME[] = "/dev/tty485";
static uint8_t devbuf[DEVBUF_SIZE];
static int devfd = 0;
static Selector* selector = NULL;

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

    Device* dev = new SONBEST_SD5110B(0x1);
    //todo: if dev=NULL

    dev->send_cmd(NULL, devfd);
    //todo: if fail

	memset(devbuf, 0, DEVBUF_SIZE);
	while(true) {
		int r = selector->select(&read_timeout);
        if(-1 == r)
        {
			cout << "select error" << endl;
		    //todo:log
            return;
		} else if (0 == r) {    //time out
            cout << "read timeout" << endl;
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
				break;
				//todo: reopen dev
			} else {
                pthread_mutex_lock(&rb_mutex);
                rbuffer->put(devbuf, devbytes);
                pthread_cond_signal(&rb_cond);
                pthread_mutex_unlock(&rb_mutex);                
				dev_log(DEVNAME, devbuf, devbytes);				
                break;
			}
        } 
	}

} 

void* serial_poll_run(void* arg)
{
	cout << "serial poll thread running" << endl;	
	
    TimerList* tmlist = (TimerList*)arg;

#ifdef TRAITS_DEBUG_GW
    list<Timer*>::iterator it;                                                                        
#endif

    devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        cout << "Open " << DEVNAME << " failed" << endl;
		//todo: log
        goto out;
    } 

	selector = new Selector();
    //todo: check selector != NULL
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

    tmlist->start(); 
	
out:
    close(devfd);

	cout << "serial thread exit" << endl;
	//todo: log	

	return 0;
}


void* serial_listen_run(void* arg)
{
	cout << "serial listen thread running" << endl;	
	
	Selector selector;

	int devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        cout << "Open " << DEVNAME << "failed" << endl;
		//todo: log
        goto out;
    } 

	selector.set_fd(devfd, READ);
	memset(devbuf, 0, DEVBUF_SIZE);

	SERIAL_RUNNING = true;	
	while(SERIAL_RUNNING) {
		if(selector.select(NULL) == -1) {
			cout << "select error" << endl;
			sleep(1);
            continue;
		} 
	
		int devbytes = 0;
		if(selector.fd_isset(devfd, READ)) {   
			devbytes = read(devfd, devbuf, DEVBUF_SIZE);
			if(devbytes <= 0) {
				cout << DEVNAME << " closed" << endl;
				//todo: log
				close(devfd);
				selector.fd_clr(devfd, READ);
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
	
out:
	cout << "serial thread exit" << endl;
	//todo: log	
   
	return 0;
}


