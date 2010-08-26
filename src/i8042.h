#ifndef I8042_H
#define I8042_H

#include "common.h"
#include "file.h"
#include "isr.h"

void module_i8042_init();

void module_i8042_cleanup();

s32int i8042_probe();

s32int i8042_read(file_t *f, u32int offset, u32int sz, u8int *buffer);

s32int i8042_write(file_t *f, u32int offset, u32int sz, u8int *buffer);

s32int i8042_open(file_t *f);

s32int i8042_close(file_t *f);

void i8042_irq(registers_t *regs);

#endif /* I8042_H */
