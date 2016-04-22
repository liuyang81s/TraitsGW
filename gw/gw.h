#ifndef TRAITS_GW_H
#define TRAITS_GW_H

#include <string>

using namespace std;

typedef enum{
	LISTEN,
	POLL,
	INVALID,
}WORK_MODE;


class TRAITS_GW
{
public:
	TRAITS_GW();
	~TRAITS_GW();	

	bool init();

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

