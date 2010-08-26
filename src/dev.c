#include "dev.h"
#include "screen.h"

dev_t *devs = 0;

void add_dev(dev_t *dev)
{
    dev->next = devs;
    devs = dev;
}

void del_dev(dev_t *dev)
{
    if (!dev)
        return;

    if (dev == devs) {
        devs = devs->next;
    } else {
        dev_t *p = devs;

        while ((p->next) && (p->next != dev))
            p = p->next;

        if (p->next == dev) {
            p->next = p->next->next;
        }
    }
    dev->next = 0;
}

dev_t *find_dev(u32int id)
{
    dev_t *p = devs;

    while (p) {
        if (p->dev_id == id)
            return p;
        p = p->next;
    }

    return 0;
}

