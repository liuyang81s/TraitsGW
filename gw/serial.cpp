#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "selector.h"
#include "serial.h"
#include "main.h"
#include "dev.h"
#include "timerlist.h"


#ifdef POLL_MODE
#include "timerlist.h"
#endif


#define DEVBUF_SIZE 128

using namespace std;

//static const char DEVNAME[] = "/dev/ttyATH0";
static const char DEVNAME[] = "/dev/ttyS0";
static uint8_t devbuf[DEVBUF_SIZE];

bool SERIAL_RUNNING = false;

void dev_log(const char* prefix, uint8_t *buf, int size)    
{
#ifdef TRAITS_DEBUG
	cout << prefix << ": ";
    cout.fill('0');
    for(int i = 0; i < size; i++)
		cout << setw(2) << hex << (uint32_t)buf[i] << ' ';
	cout << endl;
#endif
}

#ifndef POLL_MODE

void* serial_run(void* arg)
{
	cout << "serial thread running" << endl;	
	
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

#else

void* serial_run(void* arg)
{
	cout << "serial thread running" << endl;	
	
	int devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        cout << "Open " << DEVNAME << "failed" << endl;
		//todo: log
        goto out;
    } 

	Selector selector;
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

#endif

