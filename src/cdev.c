#include "cdev.h"
#include "screen.h"

cdev_t *cdevs = 0;

void add_cdev(cdev_t *cdev)
{
    cdev->next = cdevs;
    cdevs = cdev;
}

void del_cdev(cdev_t *cdev)
{
    if (!cdev)
        return;

    if (cdev == cdevs) {
        cdevs = cdevs->next;
    } else {
        cdev_t *p = cdevs;

        while ((p->next) && (p->next != cdev))
            p = p->next;

        if (p->next == cdev) {
            p->next = p->next->next;
        }
    }
    cdev->next = 0;
}

cdev_t *find_cdev(u32int id)
{
    cdev_t *p = cdevs;

    while (p) {
        if ((p->first_id <= id) && (id < p->first_id+p->count))
            return p;
        p = p->next;
    }

    return 0;
}

