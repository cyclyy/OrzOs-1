#ifndef CDEV_H
#define CDEV_H 

#include "common.h"
#include "module.h"
#include "file.h"

typedef struct dev_struct {
    u32int dev_id;
    module_t *m;
    s32int (*open)(file_t *f);
    s32int (*close)(file_t *f);
    s32int (*read)(file_t *f, u32int offset, u32int sz, u8int *buffer);
    s32int (*write)(file_t *f, u32int offset, u32int sz, u8int *buffer);
    struct dev_struct *next;
    void *priv;
} dev_t;

extern dev_t *devs;

void add_dev(dev_t *dev);

void del_dev(dev_t *dev);

dev_t *find_dev(u32int id);

#endif /* CDEV_H */
