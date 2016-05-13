#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "crc16.h"
#include "sonbest-SD5110B.h"

using namespace std;

SONBEST_SD5110B::SONBEST_SD5110B()
{
    _addr = 0x1;
}

SONBEST_SD5110B::SONBEST_SD5110B(const uint8_t addr)
{
   if(addr == 0)
      ;//todo throw exception

   _addr = addr; 
}

SONBEST_SD5110B::~SONBEST_SD5110B()
{

}

uint8_t SONBEST_SD5110B::get_addr() const
{
    return _addr;
}


void SONBEST_SD5110B::set_addr(const uint8_t addr)
{
    if(0 == addr)
        return;

    _addr = addr;
}

bool SONBEST_SD5110B::send_cmd(uint8_t* cmd, int fd)
{
    if(0 >= fd)
        return false;

    cout << "SONBEST_SD5110B send_cmd" << endl;

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

    //write cmd to /dev/tty*
    int writebytes = write(fd, read_cmd, 8);
    if(writebytes < 8){
        //todo: failed, log
        cout << "writebytes = " << writebytes << endl;                                
        return false;           
    }

    return true;
}

int SONBEST_SD5110B::recognize_packet(uint8_t* buf)
{
    //todo: more careful
    return 9;
}

