#ifndef INITRDFS_H
#define INITRDFS_H 

#include "common.h"
#include "vfs.h"

#define INITRDFS_MAGIC 0xFD

typedef struct {
    u32int nfiles;
} initrdfs_header_t;

typedef struct {
    u8int magic;
    char name[MAX_NAME_LEN]; 
    u32int offset;
    u32int length;
} __attribute__((packed)) initrdfs_file_header_t;

void module_initrdfs_init();
void module_initrdfs_cleanup();


#endif /* INITRDFS_H */
