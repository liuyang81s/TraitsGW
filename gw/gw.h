#ifndef TRAITS_GW_H
#define TRAITS_GW_H

#include <string>
#include <stdint.h>
#include "serial.h"
#include "timerlist.h"
#include "packetfilemgr.h"
#include "traits.h"

using namespace std;


extern bool GW_RUNNING;
extern bool HB_RUNNING;

void* gw_run(void* arg);
void* hb_run(void* arg);

typedef enum{
    SEND_NOTHING = 0,   //no need to send any            
    SEND_ASCII,         //send data in ascii format         
    SEND_HEX,           //send data in hex format
    SEND_INVALID,
}SEND_TYPE;

typedef enum{
    PROTO_NONE = 0,     //protocol NOT found
    PROTO_MODBUS,       //modbus protocol
    PROTO_OTHER,        //protocol but not modbus
    PROTO_INVALID,
}PROTO_TYPE;

typedef enum{
    PLAN_NONE = 0,     //there is no plan
    PLAN_UPDATE,       //update to new plan
    PLAN_REMAIN,       //remain old plan
    PLAN_INVALID,
}PLAN_MODE;


class TraitsGW
{
public:
	TraitsGW();
	TraitsGW(const string& url);
	~TraitsGW();	

	TRAITScode request_init();
	TRAITScode heartbeat();
	TRAITScode report(uint8_t *packet, const int size);

    UART_MODE get_uart_mode() const;
    SEND_TYPE get_send_type() const;
    PROTO_TYPE get_proto_type() const;
    PLAN_MODE get_plan_mode() const;
	TimerList* get_timerlist() const;		

protected:
	void init();
	string get_self_id();

    TRAITScode init_response_handler(const string& response);
    TRAITScode data_response_handler(const string& response);
    TRAITScode hb_response_handler(const string& response);

	/* device related */
	string gage_name;
	string vendor;
	string gage_type;
	string gage_no;

	string self_id;
	string server_url;
	UART_MODE uart_mode;
	SEND_TYPE send_type;
    PROTO_TYPE proto;

    PLAN_MODE plan_mode;
	TimerList* tmlist;		
    uint8_t collect_cycle;    

	PacketFileMgr pfmgr;
};

#endif

