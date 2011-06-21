#include "devfs.h"
#include "vfs.h"
#include "util.h"
#include "device.h"
#include "kmm.h"

s64int devfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
s64int devfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs);

s64int devfsRoot(struct FileSystem *fs);
s64int devfsOpen(struct FileSystem *fs, s64int id, s64int *openId);
s64int devfsClose(struct FileSystem *fs, s64int id);
s64int devfsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info);
s64int devfsRead(struct FileSystem *fs, s64int id, u64int offset, u64int size, char *buffer);
s64int devfsReaddir(struct FileSystem *fs, s64int id, u64int bufSize, char *buf);
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
    .write = 0,
    .readdir = devfsReaddir,
    .finddir = devfsFinddir,
    .stat = devfsStat,
    .mkobj = devfsCreateObject,
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
    (*fs)->data = fsData;
    return 0;
}

s64int devfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs)
{
    struct DevFSData *fsData;

    fsData = (struct DevFSData*)fs->data;
    kFree(fsData);
    return 0;
}

s64int devfsRoot(struct FileSystem *fs)
{
    struct DevFSData *fsData;

    fsData = (struct DevFSData *)fs->data;
    return (u64int)fsData->root - HEAP_START_ADDR;
}

s64int devfsOpen(struct FileSystem *fs, s64int id, s64int *openId)
{
    *openId = id;
    return 0;
}

s64int devfsClose(struct FileSystem *fs, s64int id)
{
    return 0;
}

s64int devfsRead(struct FileSystem *fs, s64int id, u64int offset, u64int size, char *buffer)
{
    struct DevFSNode *node;
    struct Device *dev;
    s64int ret;

    node = (struct DevFSNode *)(id + HEAP_START_ADDR);
    if (node->type == DEVFS_TYPE_OBJ) {
        dev = findDevice(node->objid);
        if (dev && dev->op->read) {
            ret = dev->op->read(dev,offset,size,buffer);
        } else
            ret = -1;
    } else 
        ret = -1;

    return ret;
}

s64int devfsReaddir(struct FileSystem *fs, s64int id, u64int bufSize, char *buf)
{
    struct DevFSNode *node;
    struct DirectoryRecord *drec;
    s64int ret, pos, n, dsize;

    node = (struct DevFSNode *)(id + HEAP_START_ADDR);
    if (node->type == DEVFS_TYPE_DIR) {
        node = node->children;
        pos = 0;
        n = 0;
        while (node) {
            drec = (struct DirectoryRecord *)(buf + pos);
            dsize = sizeof(struct DirectoryRecord) + strlen(node->name);
            if (pos + dsize <= bufSize) {
                drec->size = dsize;
                strcpy(drec->buffer, node->name);
                pos += dsize;
                n++;
            } else 
                break;
            node = node->next;
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

    node = (struct DevFSNode *)(id + HEAP_START_ADDR);
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
            ret = (u64int)(node) - HEAP_START_ADDR;
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

    node = (struct DevFSNode *)(id + HEAP_START_ADDR);
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

