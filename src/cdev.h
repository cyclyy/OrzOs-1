#ifndef CDEV_H
#define CDEV_H 

#include "common.h"
#include "module.h"
#include "vfs.h"

typedef struct cdev_struct {
    u32int first_id;
    u32int count;
    module_t *m;
    u32int (*read)(vnode_t *vnode, u32int offset, u32int sz, u8int *buffer);
    u32int (*write)(vnode_t *vnode, u32int offset, u32int sz, u8int *buffer);
    void (*open)(struct vnode *vnode);
    void (*close)(struct vnode *vnode);
    struct cdev_struct *next;
} cdev_t;

extern cdev_t *cdevs;

void add_cdev(cdev_t *cdev);

void del_cdev(cdev_t *cdev);

cdev_t *find_cdev(u32int id);

#endif /* CDEV_H */
