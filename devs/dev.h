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
	Device() { }
	virtual ~Device() { }
    
    virtual bool send_cmd(uint8_t* cmd, int fd) = 0; 
	/* 识别缓存中一帧完整报文
	 * buf - 缓存地址
	 * szie - 缓存有效字节数
	 * 返回值：0,无完整报文;>0,报文长度
	 */
	virtual int recognize_packet(uint8_t* buf, int size) = 0;

	/*
	 * -1 for non-fixed length
	 */	
	virtual int get_packet_size() = 0;

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
	TestDevice() { }
	virtual ~TestDevice() { }
	
    bool send_cmd(uint8_t* cmd, int fd) { return true; }
	int recognize_packet(uint8_t* buf, int size) { return 1; }
	int get_packet_size() {return 1; }

protected:

};

#endif

