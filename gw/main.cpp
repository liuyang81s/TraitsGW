#include <iostream>
#include <json-c/json.h>
#include <pthread.h>
#include <unistd.h>

#include "http.h"
#include "gw.h"
#include "serial.h"


using namespace std;

int main()
{
	pthread_t t_serial;
#if 0
	//TRAITS_GW gw;
	TRAITS_GW gw("http://traits.imwork.net:10498/AnalyzeServer/system/");

	gw.init();
	cout << endl << endl;

	gw.hb();
	cout << endl << endl;

	gw.data();
#else
	int rc = pthread_create(&t_serial, NULL, serial_routine, NULL);
	if(rc){
		cout << "ERR: pthread_create failed with " << rc << endl;
		return -rc;
	}

	sleep(100 * 30);
	SERIAL_RUNNING = false;

	pthread_join(t_serial, NULL);
	cout << "main thread exit" << endl;
#endif

	return 0;
}


