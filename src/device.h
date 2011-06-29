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
    s64int (*open)(struct Device *dev);
    s64int (*close)(struct Device *dev);
    s64int (*read)(struct Device *dev, u64int offset, u64int size, char *buffer);
    s64int (*write)(struct Device *dev, u64int offset, u64int size, char *buffer);
    s64int (*ioctl)(struct Device *dev, s64int request, u64int size, void *data);
};

void addDevice(struct Device *dev);

void removeDevice(struct Device *dev);

struct Device *findDevice(u64int id);

struct Device *getAllDevices();

#endif /* DEVICE_H */
