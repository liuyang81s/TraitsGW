#include <iostream>

#include "dev.h"

using namespace std;

Device::Device()
{

}


Device::~Device()
{

}


TestDevice::TestDevice()
{

}

TestDevice::~TestDevice()
{
	
}

int TestDevice::recognize_packet(uint8_t* buf)
{
	cout << "recognize a packet" << endl;	

	return 1;
}

