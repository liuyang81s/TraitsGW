#ifndef TRAITS_DEV_H
#define TRAITS_DEV_H

#include <stdint.h>
#include <string>

using namespace std;

typedef enum {
	NONE,
	ODD,
	EVEN,
}PARITY_MODE;


class Device
{
public:
	Device();
	virtual ~Device();
		
	virtual int recognize_packet(uint8_t* buf) = 0;
	
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
	
	int recognize_packet(uint8_t* buf);

protected:

};

#endif

