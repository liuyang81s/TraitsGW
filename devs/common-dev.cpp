#include <iostream>
#include <stdint.h>
#include <unistd.h>

#include "common-dev.h"

using namespace std;

CommonDev* CommonDev::p_instance = 0;

CommonDev::CommonDev()
{

}

CommonDev::~CommonDev()
{

}

CommonDev* CommonDev::instance()
{
	if(NULL == p_instance) {
		static CommonDev instance;
		p_instance = &instance;
	}

	return p_instance;
}

bool CommonDev::send_cmd(uint8_t* cmd, int size, int fd)
{
	if(NULL == cmd || 0 >= size || 0 >= fd)
		return false;

#if 1
    cout << "CommonDev send_cmd" << endl;
	for(int i = 0; i < size; i++){
		cout << (int)cmd[i] << " ";
	}
	cout << endl;
#endif

    //write cmd to /dev/tty*
    int writebytes = write(fd, cmd, size);
    if(writebytes < size){
        //todo: failed, log
        cout << "writebytes = " << writebytes << endl;                                
        return false;           
    }

    return true;
}

int CommonDev::recognize_packet(uint8_t* buf, int size)
{
	if((size < packet_size) || (NULL == buf))
		return 0;

	if(packet_size > 0)
	    return packet_size;	//fixed length packet
	else
		return size;	//non-fixed length packet, get all bytes of the buf
}

