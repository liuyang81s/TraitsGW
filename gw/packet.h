#ifndef TRAITS_PACKET_H
#define TRAITS_PACKET_H

#include <string>
#include <list>

//abstract response packet
class TraitsPacket
{
public:
	TraitsPacket();
	virtual ~TraitsPacket();

	virtual bool handle() = 0;

protected:
	int code;
	string errMsg;

};

//init response packet
class TraitsInitPacket : public TraitsPacket
{
public:
	TraitsInitPacket();
	virtual ~TraitsInitPacket();

	bool handle();
	
protected:
	string time;
	int modbusType;	//1:modbus, 2:not modbus
	int isListen;	//1:listen, 0:poll
	int sendType;	//1:hex, 2:ascii
	//string sendContent;
	int isPlan;		//0:have no plan, 
					//1:plan, no need to update
					//2:plan, need to update
	std::list<string> planklist;
	bool collectCycle;	

};

//data response packet
class TraitsDataPacket : public TraitsPacket
{
public:
	TraitsDataPacket()
	virtual ~TraitsDataPacket();

protected:

};


//heartbeat response packet
class TraitsHBPacket : public TraitsPacket
{
public:
	TraitsHBPacket();
	virtual ~TraitsHBPacket();

protected:
	string time;
	int isPlan;		//0:have no plan, 
					//1:plan, no need to update
					//2:plan, need to update
	std::list<string> planklist;
	bool collectCycle;	
};

#endif

