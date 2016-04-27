#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "selector.h"
#include "serial.h"
#include "main.h"

using namespace std;

#define DEVBUF_SIZE 128

static const char DEVNAME[] = "/dev/ttyATH0";
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

void* serial_routine(void* arg)
{
	cout << "serial_thread running" << endl;	

	memset(devbuf, 0, DEVBUF_SIZE);

	int devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        cout << "Open " << DEVNAME << "failed" << endl;
		//todo: log
        return 0;
    } 

	Selector selector;
	selector.set_fd(devfd, READ);

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
				dev_log(DEVNAME, devbuf, devbytes);				
			}
			
        } 

		
	}
	
	cout << "serial thread exit" << endl;
	//todo: log	

	return 0;
}


