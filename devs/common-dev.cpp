#include <iostream>
#include <stdint.h>
#include <unistd.h>

#include "common-dev.h"

using namespace std;

CommonDev::CommonDev()
{

}

CommonDev::~CommonDev()
{

}

bool CommonDev::send_cmd(uint8_t* cmd, int size, int fd)
{
	if(NULL == cmd || 0 >= size || 0 >= fd)
		return false;

    cout << "CommonDev send_cmd" << endl;

#if 0
    //make 'read data' packet
    static uint8_t read_cmd[8];
    memset(read_cmd, 0, 8);     
    
    read_cmd[0] = _addr;
    read_cmd[1] = 0x03;
    read_cmd[2] = 0x0;
    read_cmd[3] = 0x0;
    read_cmd[4] = 0x0;
    read_cmd[5] = 0x2;
 
    uint16_t crc = CRC16(read_cmd, 6);
    read_cmd[6] = crc & 0xff;
    read_cmd[7] = crc >> 8;
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
#if 1
	cout << "CommonDev size = " << size<< endl;
#endif

	if((size < packet_size) || (NULL == buf))
		return 0;

	if(packet_size > 0)
	    return packet_size;	//fixed length packet
	else
		return 1;	//non-fixed length packet
}

