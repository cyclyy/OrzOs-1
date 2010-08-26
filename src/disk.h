#ifndef DISK_H
#define DISK_H 

#include "common.h"

typedef struct disk_struct disk_t; 
struct disk_struct {
    u32int dev_id;
    char *name;
    u32int blk_size;
    u32int nr_blks;
    u32int (*read_blks)(disk_t *disk, u32int start_blk, u32int nr_blks, u8int *buf);
    u32int (*write_blks)(disk_t *disk, u32int start_blk, u32int nr_blks, u8int *buf);
    disk_t *next;
    void *priv;
};

typedef struct partition_struct partition_t;
struct partition_struct {
    disk_t *disk;
    u32int start_blk;
    u32int nr_blks;
    partition_t *next;
};

extern disk_t *disks;

// create and call add_dev(dev_t *disk_dev), handle partitions
void add_disk(disk_t *disk);

void del_disk(disk_t *disk);

#endif /* DISK_H */
