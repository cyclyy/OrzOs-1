#include "disk.h"
#include "device.h"
#include "kmm.h"
#include "util.h"
#include "vfs.h"

struct Disk *disks = 0;

static s64int doDiskRead(struct Disk *d, u64int offset, u64int size, char *buf)
{
    if (!d || !size)
        return 0;

    // check offset and size's range
    if ((offset<0) || (offset+size>d->nBlocks*d->blockSize)) {
        return 0;
    }

    char *tmp_buf = (char*)kMalloc(d->blockSize);
    u64int len, ret;
    ret = size;

    // read first block if offset not blockSize aligned
    if (offset & (d->blockSize-1)) {
        u64int real_offset = offset & ~(d->blockSize-1);
        len = MIN(size, real_offset+d->blockSize-offset);
        d->readBlocks(d, real_offset/d->blockSize, 1, tmp_buf);
        memcpy(buf, tmp_buf+(offset-real_offset), len);
        buf += len;
        offset += len;
        size -= len;
    }

    // now offset blockSize aligned, read middle blocks
    if (size >= d->blockSize) {
        d->readBlocks(d, offset/d->blockSize, size/d->blockSize, buf);
        len = size & ~(d->blockSize-1);
        buf += len;
        offset += len;
        size -= len;
    }

    // read last block if needed
    if (size>0) {
        d->readBlocks(d, offset/d->blockSize, 1, tmp_buf);
        memcpy(buf, tmp_buf, size);
    }

    kFree(tmp_buf);

    return ret;
}

static u64int doDiskWrite(struct Disk *d, u64int offset, u64int size, char *buf)
{
    if (!d || !size)
        return 0;

    // check offset and size's range
    if ((offset<0) || (offset+size>d->nBlocks*d->blockSize)) {
        return 0;
    }

    char *tmp_buf = (char*)kMalloc(d->blockSize);
    u64int len, ret;
    ret = size;


    // read first block and modify it, then write back, if offset not blockSize aligned
    if (offset & (d->blockSize-1)) {
        u64int real_offset = offset & ~(d->blockSize-1);
        d->readBlocks(d, real_offset/d->blockSize, 1, tmp_buf);
        len = MIN(size, real_offset+d->blockSize-offset);
        memcpy(tmp_buf+offset-real_offset, buf, len);
        d->writeBlocks(d, real_offset/d->blockSize, 1, tmp_buf);
        buf += len;
        offset += len;
        size -= len;
    }

    // now offset is blockSize-aligned, write middle blocks
    if (size >= d->blockSize) {
        d->writeBlocks(d, offset/d->blockSize, size/d->blockSize, buf);
        len = size & ~(d->blockSize-1);
        buf += len;
        offset += len;
        size -= len;
    }

    // read last block, modify it, write back if needed
    if (size>0) {
        d->readBlocks(d, offset/d->blockSize, 1, tmp_buf);
        memcpy(tmp_buf, buf, size);
        d->writeBlocks(d, offset/d->blockSize, 1, tmp_buf);
    }

    kFree(tmp_buf);

    return ret;
}

static s64int diskOpen(struct VNode *node)
{
    return 0;
}

static s64int diskClose(struct VNode *node)
{
    return 0;
}

static s64int diskRead(struct VNode *node, u64int size, char *buf)
{
    struct Disk *d = (struct Disk*)((struct Device*)node->priv)->priv;
    s64int ret;
    ret = doDiskRead(d,node->offset,size,buf);
    if (ret>0) {
        node->offset += ret;
    }
    return ret;
}

static s64int diskWrite(struct VNode *node, u64int size, char *buf)
{
    struct Disk *d = (struct Disk*)((struct Device*)node->priv)->priv;
    s64int ret;
    ret = doDiskWrite(d,node->offset,size,buf);
    if (ret>0) {
        node->offset += ret;
    }
    return ret;
}

static struct DeviceOperation diskOps = {
    .open  = &diskOpen,
    .close = &diskClose,
    .read  = &diskRead,
    .write = &diskWrite
};

static s64int partRead(struct VNode *node, u64int size, char *buf)
{
    struct Partition *p = (struct Partition*)((struct Device*)node->priv)->priv;
    struct Disk *d = p->disk;
    s64int offset, ret;

    // get disk offset from partition's startBlock;
    offset = node->offset + d->blockSize*p->startBlock;
    // don't over-write
    if (size>p->nBlocks*d->blockSize) {
        size = p->nBlocks*d->blockSize;
    }
    ret = doDiskRead(d,node->offset,size,buf);
    if (ret>0) {
        node->offset += ret;
    }
    return ret;
}

static s64int partWrite(struct VNode *node, u64int size, char *buf)
{
    struct Partition *p = (struct Partition*)((struct Device*)node->priv)->priv;
    struct Disk *d = p->disk;
    s64int ret, offset;

    // get disk offset from partition's startBlock;
    offset = node->offset + d->blockSize*p->startBlock;
    // don't over-write
    if (size>p->nBlocks*d->blockSize) {
        size = p->nBlocks*d->blockSize;
    }
    ret = doDiskWrite(d,node->offset,size,buf);
    if (ret>0) {
        node->offset += ret;
    }
    return ret;
}

static struct DeviceOperation partOps = {
    .open  = 0,
    .close = 0,
    .read  = &partRead,
    .write = &partWrite,
};

static void addPartition(struct Disk *disk, u8int type, u32int startBlock, u32int nBlocks, u64int *start_part_no)
{
    char deviceName[MAX_NAME_LEN];
    char *ebr;
    s64int n = 0, i = 0;

    // prevent stack overflow
    if (*start_part_no > 10)
        return;
    if (type==0x5 || type==0xf) {
        // extended partition
        ebr = (char*)kMalloc(512);
        //   read EBR
        n = doDiskRead(disk,0,512,ebr);
        //   check EBR signature
        if (((ebr[510] & 0xff) != 0x55) || ((ebr[511] & 0xff) != 0xaa))
            goto err;
        //   get the offset of four 16-byte entries(primary partition)
        ebr += 446;
        for (i=0; i<2; i++) {
            // ebr[4]:type byte, ebr[8-0xb]:startBlock dword, ebr[0xc-0xf]:nBlocks dword;
            // *(u64int*)(ebr+8) is relative offset 
            // type, startBlock, nBlocks, first partition number;
            addPartition(disk, ebr[4], startBlock+*(u32int*)(ebr+8), *(u32int*)(ebr+0xc), start_part_no);
            ebr += 16;
        }
err:
        kFree(ebr);
    } else {
        // not empty and valid
        if (nBlocks) {
            // create struct Partition
            struct Partition *part = (struct Partition*)kMalloc(sizeof(struct Partition));
            part->disk = disk;
            part->startBlock = startBlock;
            part->nBlocks = nBlocks;
            *start_part_no += 1;
            // create struct Device
            struct Device *d = (struct Device*)kMalloc(sizeof(struct Device));
            memset(d,0,sizeof(struct Device));
            d->id = disk->id + *start_part_no;
            d->op = &partOps;
            d->priv = part;
            sprintf(deviceName, "Device:/Disk%dPart%d", (disk->id & 0xff00) >> 8, *start_part_no);
            vfsCreateObject(deviceName,d->id);
            printk("%s\n",deviceName);
            addDevice(d);
        }
    }
}

// create and call addDisk(struct Disk *diskDisk), handle partitions
void addDisk(struct Disk *disk)
{
    char deviceName[MAX_NAME_LEN];
    char mbr[512];
    char *p;
    u64int n, i, nr_parts = 0;

    // add disk to disk list.
    if (!disks) {
        disks = disk;
    } else {
        struct Disk *p = disk;
        while (p->next) 
            p = p->next;
        p->next = disk;
    }
    disk->next = 0;

    // create struct Device, add it to device list
    struct Device *d = (struct Device*)kMalloc(sizeof(struct Device));
    memset(d,0,sizeof(struct Device));
    d->id = disk->id;
    d->op = &diskOps;
    d->priv = disk;
    addDevice(d);
    sprintf(deviceName, "Device:/Disk%d", (disk->id & 0xff00) >> 8);
    vfsCreateObject(deviceName,d->id);
    printk("%s\n", deviceName);

    // search  partitions and call add_dev;
    //   read MBR
    n = doDiskRead(disk,0,512,mbr);
    if (n < 512) {
        return;
    }
    //   check MBR signature
    if (((mbr[510] & 0xff) != 0x55) || ((mbr[511] & 0xff) != 0xaa)) {
        return;
    }
    //   get the offset of four 16-byte entries(primary partition)
    p = mbr;
    p += 446;
    for (i=0; i<4; i++) {
        // p[4]:type byte, p[8-0xb]:startBlock dword, p[0xc-0xf]:nBlocks dword;
        // type, startBlock, nBlocks, first partition number;
        addPartition(disk, p[4], *(u32int*)(p+8), *(u32int*)(p+0xc), &nr_parts);
        p += 16;
    }
}

void removeDisk(struct Disk *disk)
{
    if (!disk)
        return;

    if (disk == disks) {
        disks = disks->next;
    } else {
        struct Disk *p = disks;

        while ((p->next) && (p->next != disk))
            p = p->next;

        if (p->next == disk) {
            p->next = p->next->next;
        }
    }
    disk->next = 0;
}

