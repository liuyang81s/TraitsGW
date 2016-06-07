#ifndef TRAITS_SONBEST_SD5110B_H
#define TRAITS_SONBEST_SD5110B_H

#include <stdint.h>
#include "dev.h"

class SONBEST_SD5110B : public Device
{
public:
    SONBEST_SD5110B();
    SONBEST_SD5110B(const uint8_t addr);
    virtual ~SONBEST_SD5110B();

    uint8_t get_addr() const;
    void set_addr(const uint8_t addr);

    bool send_cmd(uint8_t* cmd, int fd);
    int recognize_packet(uint8_t* buf, int size);
	int get_packet_size();

	const static int DATA_PACKET_SIZE;

protected:
    uint8_t _addr;
};


#endif

