#ifndef TRAITS_GW_H
#define TRAITS_GW_H

#include <string>
#include <stdint.h>

#include "serial.h"

using namespace std;

extern bool GW_RUNNING;
extern bool HB_RUNNING;

void* gw_run(void* arg);
void* hb_run(void* arg);

class TraitsGW
{
public:
	TraitsGW();
	TraitsGW(string url);
	~TraitsGW();	

	bool init();
	bool heartbeat();
	bool report(uint8_t *packet, int size);

protected:

	/* device related */
	string gage_name;
	string factory;
	string gage_type;
	string gage_no;
	string gage_id;

	string server_url;
	WORK_MODE mode;
	int send_type;
};


#endif

