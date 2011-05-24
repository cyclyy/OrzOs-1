#ifndef PAGING_H
#define PAGING_H

#include "common.h"

#define PAGE_SIZE 0x1000
#define PAGE_MASK 0xfffffffffffff000

#define PFAULT_HANDLED   0
#define PFAULT_EACCESS   1
#define PFAULT_ERDONLY   2

#define INVLPG(X) {asm volatile("invlpg %0"::"m"(*(char*)X)); }
#define ALIGN(X) (((X)&(PAGE_SIZE-1)) ? (((X)&PAGE_MASK)+PAGE_SIZE) : X)

typedef struct {
    u32int present : 1;
    u32int rw : 1;
    u32int us : 1;
    u32int rsvd1 : 2;
    u32int accessed : 1;
    u32int dirty : 1;
    u32int rsvd2 : 2;
    u32int avail : 3;
    u32int addr : 20;
} page_t;

typedef struct {
    page_t pages[1024];
} page_table_t;

typedef struct {
    page_table_t *page_tables[1024];
    u32int table_addr[1024];
    u32int addr;
} page_directory_t;

void init_paging();

void switch_page_directory(page_directory_t *dir);

page_t *get_page(u32int vaddr, u32int make, page_directory_t *dir);

void alloc_frame(page_t *p, u32int is_user, u32int writable);

void free_page(page_t *page);

void copy_page(page_t *src, page_t *dst);

page_table_t *clone_table(page_table_t *table, u32int *phys);

page_directory_t *clone_directory(page_directory_t *dir);

//void dump_tables(page_directory_t *dir);

#endif /* PAGING_H */
