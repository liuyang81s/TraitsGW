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


#define DEVBUF_SIZE 128

using namespace std;

//static const char DEVNAME[] = "/dev/ttyATH0";
static const char DEVNAME[] = "/dev/ttyS0";
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

void serial_onTime(int sock, short event, void *arg)
{
    static struct timeval read_timeout = {5, 0};

    if(devfd == 0)
        return;

    cout << "onTime" << endl;


    uint8_t cmd[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xc4, 0x0b};
    
    int count = sizeof(cmd);
    int writebytes = write(devfd, cmd, count);
    if(writebytes < count)
    {
        //todo: failed, log
        cout << "writebytes = " << writebytes << endl;
        cout << "write failed" << endl;
        return ;
    }


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

void* serial_run(void* arg)
{
	cout << "serial thread running" << endl;	
	
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    
    Timer tm(tv, 5);
    tm.onTime = serial_onTime;
	
    TimerList* tmlist = new TimerList();

    devfd = open(DEVNAME, O_RDWR);
    if ( devfd == -1 ) { 
        cout << "Open " << DEVNAME << "failed" << endl;
		//todo: log
        goto out;
    } 

	selector = new Selector();
    //todo: check selector != NULL
	selector->set_fd(devfd, READ);

    tmlist->init();
    tmlist->add_timer(&tm);

    tmlist->start(); 
	
out:
    delete tmlist;
    close(devfd);

	cout << "serial thread exit" << endl;
	//todo: log	

	return 0;
}

