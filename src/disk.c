#include "disk.h"
#include "dev.h"
#include "kheap.h"
#include "screen.h"
#include "file.h"

struct Disk *disks = 0;

static u64int doDiskRead(struct Disk *d, u64int offset, u64int sz, u8int *buf)
{
    if (!d || !sz)
        return 0;

    // check offset and sz's range
    if ((offset<0) || (offset+sz>d->nr_blks*d->blk_size)) {
        return 0;
    }

    u8int *tmp_buf = (u8int*)kMalloc(d->blk_size);
    u64int len;
    u64int size = sz;

    // read first block if offset not blk_size aligned
    if (offset & (d->blk_size-1)) {
        u64int real_offset = offset & ~(d->blk_size-1);
        len = MIN(sz, real_offset+d->blk_size-offset);
        d->read_blks(d, real_offset/d->blk_size, 1, tmp_buf);
        memcpy(buf, tmp_buf+(offset-real_offset), len);
        buf += len;
        offset += len;
        sz -= len;
    }

    // now offset blk_size aligned, read middle blocks
    if (sz >= d->blk_size) {
        d->read_blks(d, offset/d->blk_size, sz/d->blk_size, buf);
        len = sz & ~(d->blk_size-1);
        buf += len;
        offset += len;
        sz -= len;
    }

    // read last block if needed
    if (sz>0) {
        d->read_blks(d, offset/d->blk_size, 1, tmp_buf);
        memcpy(buf, tmp_buf, sz);
    }

    kfree(tmp_buf);

    return size;
}

static u64int doDiskWrite(struct Disk *d, u64int offset, u64int sz, u8int *buf)
{
    if (!d || !sz)
        return 0;

    // check offset and sz's range
    if ((offset<0) || (offset+sz>d->nr_blks*d->blk_size)) {
        return 0;
    }

    u8int *tmp_buf = (u8int*)kMalloc(d->blk_size);
    u64int len;
    u64int size = sz;


    // read first block and modify it, then write back, if offset not blk_size aligned
    if (offset & (d->blk_size-1)) {
        u64int real_offset = offset & ~(d->blk_size-1);
        d->read_blks(d, real_offset/d->blk_size, 1, tmp_buf);
        len = MIN(sz, real_offset+d->blk_size-offset);
        memcpy(tmp_buf+offset-real_offset, buf, len);
        d->write_blks(d, real_offset/d->blk_size, 1, tmp_buf);
        buf += len;
        offset += len;
        sz -= len;
    }

    // now offset is blk_size-aligned, write middle blocks
    if (sz >= d->blk_size) {
        d->write_blks(d, offset/d->blk_size, sz/d->blk_size, buf);
        len = sz & ~(d->blk_size-1);
        buf += len;
        offset += len;
        sz -= len;
    }

    // read last block, modify it, write back if needed
    if (sz>0) {
        d->read_blks(d, offset/d->blk_size, 1, tmp_buf);
        memcpy(tmp_buf, buf, sz);
        d->write_blks(d, offset/d->blk_size, 1, tmp_buf);
    }

    kfree(tmp_buf);

    return size;
}

static s32int diskOpen(file_t *f)
{
    return 0;
}

static s32int diskClose(file_t *f)
{
    return 0;
}

static s32int diskRead(file_t *f, u64int offset, u64int sz, u8int *buf)
{
    struct Disk *d = (struct Disk*)((dev_t*)f->priv)->priv;
    return doDiskRead(d,offset,sz,buf);
}

static s32int disk_write(file_t *f, u64int offset, u64int sz, u8int *buf)
{
    struct Disk *d = (struct Disk*)((dev_t*)f->priv)->priv;
    return doDiskWrite(d,offset,sz,buf);
}

struct file_operations disk_fops = {
    .open  = &disk_open,
    .close = &disk_close,
    .read  = &disk_read,
    .write = &disk_write
};

static s32int part_read(file_t *f, u64int offset, u64int sz, u8int *buf)
{
    partition_t *p = (partition_t*)((dev_t*)f->priv)->priv;
    struct Disk *d = p->disk;

    // get disk offset from partition's start_blk;
    offset += d->blk_size*p->start_blk;
    // don't over-read
    if (sz>p->nr_blks*d->blk_size) {
        sz = p->nr_blks*d->blk_size;
    }
    return doDiskRead(d,offset,sz,buf);
}

static s32int part_write(file_t *f, u64int offset, u64int sz, u8int *buf)
{
    partition_t *p = (partition_t*)((dev_t*)f->priv)->priv;
    struct Disk *d = p->disk;

    // get disk offset from partition's start_blk;
    offset += d->blk_size*p->start_blk;
    // don't over-write
    if (sz>p->nr_blks*d->blk_size) {
        sz = p->nr_blks*d->blk_size;
    }
    return doDiskWrite(d,offset,sz,buf);
}

struct file_operations part_fops = {
    .open  = 0,
    .close = 0,
    .read  = &part_read,
    .write = &part_write
};

void add_partition(struct Disk *disk, u8int type, u64int start_blk, u64int nr_blks, u64int *start_part_no)
{
    // prevent stack overflow
    if (*start_part_no > 10)
        return;
    if (type==0x5 || type==0xf) {
        // extended partition
        u8int *ebr = (u8int*)kMalloc(512);
        u64int n, i = 0;
        //   read EBR
        n = doDiskRead(disk,0,512,ebr);
        if (n != 512)
            goto err;
        //   check EBR signature
        if ((ebr[510] != 0x55) || (ebr[511] != 0xaa))
            goto err;
        //   get the offset of four 16-byte entries(primary partition)
        ebr += 446;
        for (i=0; i<2; i++) {
            // ebr[4]:type byte, ebr[8-0xb]:start_blk dword, ebr[0xc-0xf]:nr_blks dword;
            // *(u64int*)(ebr+8) is relative offset 
            // type, start_blk, nr_blks, first partition number;
            add_partition(disk, ebr[4], start_blk+*(u64int*)(ebr+8), *(u64int*)(ebr+0xc), start_part_no);
            ebr += 16;
        }
err:
        kfree(ebr);
    } else {
        // not empty and valid
        if (nr_blks) {
            // create partition_t
            partition_t *part = (partition_t*)kMalloc(sizeof(partition_t));
            part->disk = disk;
            part->start_blk = start_blk;
            part->nr_blks = nr_blks;
            *start_part_no += 1;
            // create dev_t
            dev_t *d = (dev_t*)kMalloc(sizeof(dev_t));
            memset(d,0,sizeof(dev_t));
            d->dev_id = disk->dev_id + *start_part_no;
            d->f_ops = &part_fops;
            d->priv = part;
            vfs_mknod("/dev/part",d->dev_id,0);
            add_dev(d);
        }
    }
}

// create and call add_disk(struct Disk *disk_disk), handle partitions
void addDisk(struct Disk *disk)
{
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

    // create dev_t, add it to device list
    dev_t *d = (dev_t*)kMalloc(sizeof(dev_t));
    memset(d,0,sizeof(dev_t));
    d->dev_id = disk->dev_id;
    d->f_ops = &disk_fops;
    d->priv = disk;
    add_dev(d);
    vfs_mknod("/dev/disk",d->dev_id,0);

    // search  partitions and call add_dev;
    u8int *mbr = (u8int*)kMalloc(512);
    u8int *p = mbr;
    u64int n, i, nr_parts = 0;
    //   read MBR
    n = doDiskRead(disk,0,512,p);
    if (n != 512)
        goto err;
    //   check MBR signature
    if ((p[510] != 0x55) || (p[511] != 0xaa))
        goto err;
    //   get the offset of four 16-byte entries(primary partition)
    p += 446;
    for (i=0; i<4; i++) {
        // p[4]:type byte, p[8-0xb]:start_blk dword, p[0xc-0xf]:nr_blks dword;
        // type, start_blk, nr_blks, first partition number;
        add_partition(disk, p[4], *(u64int*)(p+8), *(u64int*)(p+0xc), &nr_parts);
        p += 16;
    }
err:
    kfree(mbr);
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

