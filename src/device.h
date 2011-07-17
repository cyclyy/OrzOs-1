#ifndef DEVICE_H
#define DEVICE_H

#include "sysdef.h"
#include "vfs.h"

struct DeviceOperation;

struct Device {
    u64int id;
    struct DeviceOperation *op;
    struct Device *next, *prev;
    void *priv;
};

struct DeviceOperation {
    s64int (*open)(struct VNode *node);
    s64int (*close)(struct VNode *node);
    s64int (*read)(struct VNode *node, u64int size, char *buffer);
    s64int (*write)(struct VNode *node, u64int size, char *buffer);
    s64int (*seek)(struct VNode *node, s64int offset, s64int pos);
    s64int (*ioctl)(struct VNode *node, s64int request, u64int size, void *data);
    s64int (*mmap)(struct VNode *node, u64int addr, u64int size, s64int flags);
    s64int (*munmap)(struct VNode *node, u64int addr);
};

void addDevice(struct Device *dev);

void removeDevice(struct Device *dev);

struct Device *findDevice(u64int id);

struct Device *getAllDevices();

#endif /* DEVICE_H */
