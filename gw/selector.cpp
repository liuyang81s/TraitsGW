#include <iostream>
#include <sys/select.h>

#include "selector.h"

Selector::Selector()
{
    maxfd = -1;

    FD_ZERO(&readfds);
    FD_ZERO(&readfdsuse);

    FD_ZERO(&writefds);
    FD_ZERO(&writefdsuse);

    FD_ZERO(&exceptfds);
    FD_ZERO(&exceptfdsuse);
}

Selector::~Selector()
{
    FD_ZERO(&readfds);
    FD_ZERO(&readfdsuse);

    FD_ZERO(&writefds);
    FD_ZERO(&writefdsuse);

    FD_ZERO(&exceptfds);
    FD_ZERO(&exceptfdsuse);
}

void Selector::set_fd(int fd, FD_TYPE type)
{
    switch(type){
        case READ:
            FD_SET(fd, &readfds);
            break;
        case WRITE:
            FD_SET(fd, &writefds);
            break;
        case EXCEPT:
            FD_SET(fd, &exceptfds);
            break;
        default:
            std::cout << "ERR: INVALID FD_TYPE: " << type << std::endl;
            return;
    }

    if(fd >= maxfd)
        maxfd = fd + 1;    
}

int Selector::fd_isset(int fd, FD_TYPE type)
{
    switch(type){
        case READ:
            return FD_ISSET(fd, &readfdsuse);
        case WRITE:
            return FD_ISSET(fd, &writefdsuse);
        case EXCEPT:
            return FD_ISSET(fd, &exceptfdsuse);
        default:
            std::cout << "ERR: INVALID FD_TYPE: " << type << std::endl;
            return 0;
    }
}

void Selector::fd_clr(int fd, FD_TYPE type)
{
    switch(type){
        case READ:
             FD_CLR(fd, &readfds);
             break;
        case WRITE:
             FD_CLR(fd, &writefds);
             break;
        case EXCEPT:
             FD_CLR(fd, &exceptfds);
             break;;
        default:
            std::cout << "ERR: INVALID FD_TYPE: " << type << std::endl;
            break;
    }

    //todo:update maxfd

    return;
}

int Selector::select(struct timeval *timeout)
{
    readfdsuse = readfds; 
    writefdsuse = writefds;
    exceptfdsuse = exceptfds;

    return ::select(maxfd, &readfdsuse, &writefdsuse, &exceptfdsuse, timeout);
}


