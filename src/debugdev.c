#include "debugdev.h"
#include "debugcon.h"
#include "device.h"
#include "vfs.h"
#include "util.h"
#include "kmm.h"

s64int debugdev_Open(struct VNode *node);
s64int debugdev_Close(struct VNode *node);
s64int debugdev_Write(struct VNode *node, u64int size, char *buffer);

static struct Device *dev = 0;

static struct DeviceOperation debugdev_Ops = {
    .open  = &debugdev_Open,
    .close = &debugdev_Close,
    .write  = &debugdev_Write,
};

void debugdev_Init()
{
    dev = (struct Device*)kMalloc(sizeof(struct Device));
    memset(dev, 0, sizeof(struct Device));
    dev->id = 0xff0000;
    dev->op  = &debugdev_Ops;
    addDevice(dev);
    vfsCreateObject("Device:/Debug",dev->id);
}

s64int debugdev_Write(struct VNode *node, u64int size, char *buffer)
{
    u64int i;
    for (i=0; i<size; i++) {
        dbgPutch(buffer[i]);
    }
    return size;
}

s64int debugdev_Open(struct VNode *node)
{
    node->priv = dev;
    return 0;
}

s64int debugdev_Close(struct VNode *node)
{
    return 0;
}

// vim: sw=4 sts=4 et tw=100
