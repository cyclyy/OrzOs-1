#include "devfs.h"
#include "vfs.h"
#include "util.h"
#include "device.h"
#include "kmm.h"

s64int devfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
s64int devfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs);

s64int devfsOpen(struct FileSystem *fs, s64int id, struct VNode *node);
s64int devfsClose(struct VNode *node);
s64int devfsRead(struct VNode *node, u64int size, char *buffer);
s64int devfsWrite(struct VNode *node, u64int size, char *buffer);
s64int devfsSeek(struct VNode *node, s64int offset, s64int pos);
s64int devfsIoControl(struct VNode *node, s64int request, u64int size, void *data);
s64int devfsMap(struct VNode *node, u64int addr, u64int size, s64int flags);
s64int devfsUnmap(struct VNode *node, u64int addr);

s64int devfsReadAsync(struct VNode *node, u64int size, char *buffer);

s64int devfsRoot(struct FileSystem *fs);
s64int devfsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info);
s64int devfsReaddir(struct VNode *node, u64int bufSize, char *buf);
s64int devfsFinddir(struct FileSystem *fs, s64int id, const char *name);
s64int devfsCreateObject(struct FileSystem *fs, s64int id, const char *name, s64int objid);

#define DEVFS_TYPE_DIR 1
#define DEVFS_TYPE_OBJ 2

struct DevFSNode {
    u8int type;
    char name[MAX_NAME_LEN];
    s64int objid;
    struct DevFSNode *children;
    struct DevFSNode *next, *prev;
};

struct DevFSData {
    struct DevFSNode *root;
};

static struct FileSystemDriverOperation driverOps = {
    .mount = devfsMount,
    .unmount = devfsUnmount,
};

static struct FileSystemOperation fsOps = {
    .root = devfsRoot,
    .open = devfsOpen,
    .close = devfsClose,
    .read = devfsRead,
    .readAsync = devfsReadAsync,
    .write = devfsWrite,
    .readdir = devfsReaddir,
    .finddir = devfsFinddir,
    .stat = devfsStat,
    .mkobj = devfsCreateObject,
    .seek = devfsSeek,
    .mmap = devfsMap,
    .munmap = devfsUnmap,
    .ioctl = devfsIoControl,
} ;

struct FileSystemDriver devfsDriver = {
    .name = "devfs",
    .op = &driverOps,
};

s64int devfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data)
{
    struct DevFSData *fsData;
    struct DevFSNode *node;

    if (source && strlen(source)) {
        return -1;
    }

    *fs = (struct FileSystem*)kMalloc(sizeof(struct FileSystem));
    memset(*fs, 0, sizeof(struct FileSystem));
    (*fs)->driver = &devfsDriver;
    (*fs)->op = &fsOps;
    fsData = (struct DevFSData *)kMalloc(sizeof(struct DevFSData));
    memset(fsData, 0, sizeof(struct DevFSData));
    node = (struct DevFSNode *)kMalloc(sizeof(struct DevFSNode));
    memset(node, 0, sizeof(struct DevFSNode));
    strcpy(node->name, "Root");
    node->type = DEVFS_TYPE_DIR;
    fsData->root = node;
    (*fs)->priv = fsData;
    return 0;
}

s64int devfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs)
{
    struct DevFSData *fsData;

    fsData = (struct DevFSData*)fs->priv;
    kFree(fsData);
    return 0;
}

s64int devfsRoot(struct FileSystem *fs)
{
    struct DevFSData *fsData;

    fsData = (struct DevFSData *)fs->priv;
    return (u64int)fsData->root - KERNEL_HEAP_START_ADDR;
}

s64int devfsOpen(struct FileSystem *fs, s64int id, struct VNode *node)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    node->fs = fs;
    node->id = id;
    node->offset = 0;
    node->size = 0;
    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->open) {
            node->priv = dev;
            ret = dev->op->open(node);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsClose(struct VNode *node)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->close) {
            ret = dev->op->close(node);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsRead(struct VNode *node, u64int size, char *buffer)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->read) {
            ret = dev->op->read(node,size,buffer);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsReadAsync(struct VNode *node, u64int size, char *buffer)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->readAsync) {
            ret = dev->op->readAsync(node,size,buffer);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsWrite(struct VNode *node, u64int size, char *buffer)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->write) {
            ret = dev->op->write(node,size,buffer);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsReaddir(struct VNode *node, u64int bufSize, char *buf)
{
    struct DirectoryRecord *drec;
    struct DevFSNode *dnode;
    s64int ret, id, pos, off, n, dsize;

    id = node->id;
    dnode = (struct DevFSNode *)(id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_DIR) {
        dnode = dnode->children;
        pos = 0;
        n = 0;
        off = 0;
        while (dnode && (off<node->offset)) {
            off++;
            dnode = dnode->next;
        }
        while (dnode) {
            drec = (struct DirectoryRecord *)(buf + pos);
            dsize = sizeof(struct DirectoryRecord) + strlen(dnode->name);
            if (pos + dsize <= bufSize) {
                drec->size = dsize;
                strcpy(drec->buffer, dnode->name);
                pos += dsize;
                n++;
                off++;
                node->offset++;
            } else 
                break;
            dnode = dnode->next;
        }
        ret = n;
    } else
        ret = -1;

    return ret;
}

s64int devfsFinddir(struct FileSystem *fs, s64int id, const char *name)
{
    struct DevFSNode *node, *fnode;
    s64int ret;

    node = (struct DevFSNode *)(id + KERNEL_HEAP_START_ADDR);
    if (node->type == DEVFS_TYPE_DIR) {
        node = node->children;
        fnode = 0;
        while (node) {
            if (strcmp(name,node->name)==0) {
                fnode = node;
                break;
            }
            node = node->next;
        }
        if (fnode)
            ret = (u64int)(node) - KERNEL_HEAP_START_ADDR;
        else
            ret = -1;
    } else 
        ret = -1;
    return ret;
}

s64int devfsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info)
{
    return 0;
}

s64int devfsCreateObject(struct FileSystem *fs, s64int id, const char *name, s64int objid)
{
    struct DevFSNode *node, *fnode;
    s64int ret;

    node = (struct DevFSNode *)(id + KERNEL_HEAP_START_ADDR);
    if (node->type == DEVFS_TYPE_DIR) {
        fnode = (struct DevFSNode *)kMalloc(sizeof(struct DevFSNode));
        memset(fnode, 0, sizeof(struct DevFSNode));
        fnode->type = DEVFS_TYPE_OBJ;
        fnode->objid = objid;
        strcpy(fnode->name, name);
        fnode->next = node->children;
        fnode->prev = 0;
        if (node->children)
            node->children->prev = fnode;
        node->children = fnode;
    } else 
        ret = -1;
    return ret;
}

s64int devfsSeek(struct VNode *node, s64int offset, s64int pos)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->seek) {
            ret = dev->op->seek(node,offset,pos);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsIoControl(struct VNode *node, s64int request, u64int size, void *data)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->ioctl) {
            ret = dev->op->ioctl(node,request,size,data);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsMap(struct VNode *node, u64int addr, u64int size, s64int flags)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->mmap) {
            ret = dev->op->mmap(node,addr,size,flags);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsUnmap(struct VNode *node, u64int addr)
{
    struct DevFSNode *dnode;
    struct Device *dev;
    s64int ret;

    dnode = (struct DevFSNode *)(node->id + KERNEL_HEAP_START_ADDR);
    if (dnode->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(dnode->objid);
        if (dev && dev->op->munmap) {
            ret = dev->op->munmap(node,addr);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}
