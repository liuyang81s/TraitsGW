#ifndef TRAITS_COMMON_DEV_H
#define TRAITS_COMMON_DEV_H

#include <stdint.h>
#include "dev.h"

class CommonDev : public Device
{
public:
	static CommonDev* instance();

    bool send_cmd(uint8_t* cmd, int size, int fd);
    int recognize_packet(uint8_t* buf, int size);

protected:
    CommonDev();
    virtual ~CommonDev();

	static CommonDev* p_instance;
};


#endif

