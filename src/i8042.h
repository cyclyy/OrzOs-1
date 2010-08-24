#ifndef I8042_H
#define I8042_H

#include "common.h"
#include "vfs.h"
#include "isr.h"

void module_i8042_init();

void module_i8042_cleanup();

u32int i8042_probe();

u32int i8042_read(struct vnode *vnode, u32int offset, u32int sz, u8int *buffer);

u32int i8042_write(struct vnode *vnode, u32int offset, u32int sz, u8int *buffer);

void i8042_open(struct vnode *vnode);

void i8042_close(struct vnode *vnode);

void i8042_irq(registers_t *regs);

#endif /* I8042_H */
