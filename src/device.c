#include "device.h"
#include "kmm.h"
#include "util.h"

struct Device *devs = 0;

void addDevice(struct Device *dev)
{
    dev->next = devs;
    devs = dev;
}

void removeDevice(struct Device *dev)
{
    if (!dev)
        return;

    if (dev == devs) {
        devs = devs->next;
    } else {
        struct Device *p = devs;

        while ((p->next) && (p->next != dev))
            p = p->next;

        if (p->next == dev) {
            p->next = p->next->next;
        }
    }
    dev->next = 0;
}

struct Device *findDevice(u64int id)
{
    struct Device *p = devs;

    while (p) {
        if (p->id == id)
            return p;
        p = p->next;
    }

    return 0;
}

struct Device *getAllDevices()
{
    return devs;
}

