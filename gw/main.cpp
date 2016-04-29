#include <iostream>
#include <json-c/json.h>
#include <pthread.h>
#include <unistd.h>

#include "http.h"
#include "gw.h"
#include "serial.h"
#include "main.h"

using namespace std;

#define RINGBUFFER_SIZE 2048

UnlockRingBuffer *rbuffer = NULL;
pthread_mutex_t rb_mutex;
pthread_cond_t  rb_cond;


int main()
{
	pthread_t t_serial;
	pthread_t t_gw;

#if 0
	//TRAITS_GW gw;
	TRAITS_GW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");

	gw.init();
	cout << endl << endl;

	gw.hb();
	cout << endl << endl;

	gw.data();
#else	
    rbuffer = new UnlockRingBuffer(RINGBUFFER_SIZE);
    if(!rbuffer->init()) {
        cout << "ringbuffer init failed, serial thread exit" << endl;
        //todo: log
        return 0;
    }  
	
	pthread_mutex_init(&rb_mutex, NULL);
	pthread_cond_init(&rb_cond, NULL);

	int rc1 = pthread_create(&t_gw, NULL, gw_run, NULL);
	if(rc1){
		cout << "ERR: pthread_create failed with " << rc1 << endl;
		return -rc1;
	}
	
	int rc2 = pthread_create(&t_serial, NULL, serial_run, NULL);
	if(rc2){
		cout << "ERR: pthread_create failed with " << rc2 << endl;
		return -rc2;
	}


	sleep(100 * 3);
	//sleep(10);

	GW_RUNNING = false;
	SERIAL_RUNNING = false;

	pthread_join(t_gw, NULL);
	pthread_join(t_serial, NULL);
	
	pthread_mutex_destroy(&rb_mutex);
	pthread_cond_destroy(&rb_cond);

	cout << "main thread exit" << endl;
#endif

	return 0;
}

