#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <os/list.h>
#include <os/syscall.h>

struct Buffer *createBuffer(void *data, int size)
{
    struct Buffer *buffer;

    buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
    memset(buffer, 0, sizeof(struct Buffer));
    buffer->tstamp = OzGetTicks();
    buffer->size = size;
    buffer->data = malloc(size);
    memcpy(buffer->data, data, size);
    INIT_LIST_HEAD(&buffer->link);

    return buffer;
}

void destroyBuffer(struct Buffer *buffer)
{
    free(buffer->data);
    free(buffer);
}
