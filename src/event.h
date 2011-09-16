#ifndef EVENT_H
#define EVENT_H

#define EVENT_IO_READ       1
#define EVENT_IO_WRITE      2

struct GenericEvent
{
    int type;
}

struct IOEvent
{
    int type;
    int fd;
    int op;
    int ret;
};


#endif /* EVENT_H */
