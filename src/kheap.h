#ifndef KHEAP_H
#define KHEAP_H

#include "common.h"
#include "order_array.h"

#define HEAP_MAGIC (0xabcde012)

#define KHEAP_START 0x10000000
#define KHEAP_MAX_END 0x3ffff000
#define KHEAP_INIT_END 0x10100000
#define KHEAP_MAX_IDX 0x20000

typedef struct {
    u32int magic;
    u32int sz; // real size between header_t and footer_t
} header_t;

typedef struct {
    u32int magic;
    header_t *header;
} footer_t;

typedef struct {
    u32int start_addr;
    u32int end_addr;
    u32int max_end_addr;
    u32int su;
    u32int rw;
    order_array_t alloc_addr;
} heap_t;

extern u32int placement_addr;

heap_t* create_heap(u32int start_addr, u32int end_addr, u32int max_end_addr, u32int supervisor, u32int writable);

// 4kb aligned
u32int kmalloc_a(u32int sz);

// phys returns physical address of allocated frame
u32int kmalloc_p(u32int sz, u32int *phys);

// aligned, phys return version
u32int kmalloc_ap(u32int sz, u32int *phys);

// normal alloc
u32int kmalloc(u32int sz);

void kfree(void *p);

#endif
