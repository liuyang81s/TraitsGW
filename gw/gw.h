#ifndef TRAITS_GW_H
#define TRAITS_GW_H

#include <string>
#include "serial.h"

using namespace std;

extern bool GW_RUNNING;

void* gw_run(void* arg);


class TRAITS_GW
{
public:
	TRAITS_GW();
	TRAITS_GW(string url);
	~TRAITS_GW();	

	bool init();
	bool hb();
	bool data();

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

