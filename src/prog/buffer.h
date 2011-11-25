#ifndef BUFFER_H
#define BUFFER_H

#include <os/list.h>

struct Buffer
{
    long tstamp;
    int size;
    void *data;
    struct ListHead link;
};

struct Buffer *createBuffer(void *data, int size);
void destroyBuffer(struct Buffer *buffer);

#endif /* BUFFER_H */
