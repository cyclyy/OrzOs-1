#ifndef EVENT_H
#define EVENT_H

#define GET_EVENT_TYPE(x) (((struct GenericEvent*)(x))->type)

#define EVENT_IO_READ       1
#define EVENT_IO_WRITE      2
#define EVENT_IO_CANCEL     3
#define EVENT_TIMER         4

struct GenericEvent
{
    int type;
};

struct IOEvent
{
    int type;
    int fd;
    int op;
    int ret;
};

struct TimerEvent
{
    int type;
};

#endif /* EVENT_H */
