#ifndef DISK_H
#define DISK_H 

#include "sysdef.h"

struct Disk {
    u64int id;
    char *name;
    u64int blockSize;
    u64int nBlocks;
    u64int (*readBlocks)(struct Disk *disk, u64int startBlock, u64int nBlocks, void *buf);
    u64int (*writeBlocks)(struct Disk *disk, u64int startBlock, u64int nBlocks, void *buf);
    struct Disk *next;
    void *priv;
};

struct Partition {
    struct Disk *disk;
    u64int startBlock;
    u64int nBlocks;
    struct Partition *next;
};

extern struct Disk *disks;

// create and call add_dev(dev_t *disk_dev), handle partitions
void addDisk(struct Disk *disk);

void removeDisk(struct Disk *disk);

#endif /* DISK_H */
