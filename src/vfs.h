#ifndef VFS_H
#define VFS_H

#include "sysdef.h"

#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

#define VNODE_FILE          0x01
#define VNODE_DIRECTORY     0x02
#define VNODE_OBJECT        0x04

struct FileSystem;
struct FileSystemDriver;

struct DirectoryRecord
{
    u64int size;
    char buffer[1];
};

struct VNodeInfo
{
    u64int size;
};

struct VNode
{
    u64int flags;
    s64int id;
    s64int offset;
    struct FileSystem *fs;
    void *priv;
};

struct FileSystemOperation
{
    s64int (*open)(struct FileSystem *fs, s64int id, struct VNode *node);
    s64int (*close)(struct VNode *node);
    s64int (*read)(struct VNode *node, u64int size, char *buffer);
    s64int (*write)(struct VNode *node, u64int size, char *buffer);
    s64int (*seek)(struct VNode *node, s64int offset, s64int pos);
    s64int (*ioctl)(struct VNode *node, s64int request, void *data, u64int size);

    s64int (*root)(struct FileSystem *fs);
    s64int (*stat)(struct FileSystem *fs, s64int id, struct VNodeInfo *ni);
    s64int (*readdir)(struct VNode *node, u64int size, char *buf);
    s64int (*finddir)(struct FileSystem *fs, s64int id, const char *name);
    s64int (*mkobj)(struct FileSystem *fs, s64int id, const char *name, s64int objid);
    s64int (*mkdir)(struct FileSystem *fs, s64int id, const char *name);
    s64int (*remove)(struct FileSystem *fs, s64int id, const char *name);
    s64int (*rmdir)(struct FileSystem *fs, s64int id, const char *name);
};

struct FileSystem
{
    struct FileSystemDriver *driver;
    struct FileSystemOperation *op;
    void *priv;
};

struct FileSystemDriverOperation
{
    s64int (*mount)(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
    s64int (*unmount)(struct FileSystemDriver *driver, struct FileSystem *fs);
};

struct FileSystemDriver
{
    char name[MAX_NAME_LEN];
    struct FileSystemDriverOperation *op;
    struct FileSystemDriver *next, *prev;
};

s64int registerFileSystemDriver(struct FileSystemDriver *fsDriver);
s64int unregisterFileSystemDriver(const char *name);

s64int vfsMount(const char *dest, const char *source, const char *fsType, u64int flags, void *data);
s64int vfsUnmount(const char *path);

s64int vfsOpen(const char *path, u64int flags, struct VNode *node);
s64int vfsClose(struct VNode *node);
s64int vfsRead(struct VNode *node, u64int size, void *buffer);
s64int vfsWrite(struct VNode *node, u64int size, char *buffer);
s64int vfsSeek(struct VNode *node, s64int offset, s64int pos);
s64int vfsState(const char *path, struct VNodeInfo *ni);
s64int vfsIoControl(struct VNode *node, s64int request, u64int size, void *data);

s64int vfsCreateObject(const char *name, s64int id);
s64int vfsCreateDirectory(const char *name);
s64int vfsRemoveDirectory(const char *name);
s64int vfsReadDirectory(struct VNode *node, u64int size, char *buf);

s64int vfsNopOpen(struct VNode *node);
s64int vfsNopClose(struct VNode *node);
s64int vfsNopSeek(struct VNode *node, s64int offset, s64int pos);

void initVFS();

#endif /* VFS_H */
