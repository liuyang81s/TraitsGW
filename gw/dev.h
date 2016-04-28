#ifndef TRAITS_DEV_H
#define TRAITS_DEV_H

#include <stdint.h>
#include <string>

using namespace std;

typedef enum {
	NONE,
	ODD,
	EVEN,
	INVALID,
}PARITY_MODE;


class Device
{
public:
	Device();
	virtual ~Device();
		
	virtual void make_packet(uint8_t* buf) = 0;
	
protected:
	string dev_name;
	string vendor;
	string type;
	string id;

	string port_name;
	int baud;
	int databits;
	int stopbits;
	PARITY_MODE parity;
};

class TestDevice : public Device
{
public:
	TestDevice();
	virtual ~TestDevice();
	
	void make_packet(uint8_t* buf);

protected:

};

#endif

