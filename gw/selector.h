#ifndef SELECTOR_H
#define SELECTOR_H

#include <sys/select.h>

typedef enum{
    READ,
    WRITE,
    EXCEPT,
    INVALIAD,
}FD_TYPE;


class Selector
{
    public:
        Selector();
        virtual ~Selector();

        int select(struct timeval *timeout);

        void set_fd(int fd, FD_TYPE type);
        int fd_isset(int fd, FD_TYPE type);
        void fd_clr(int fd, FD_TYPE type);

    protected:
        fd_set readfds;
        fd_set readfdsuse;

        fd_set writefds;
        fd_set writefdsuse;

        fd_set exceptfds;
        fd_set exceptfdsuse;

        int maxfd;
};

#endif

