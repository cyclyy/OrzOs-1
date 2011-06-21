#ifndef VFS_H
#define VFS_H

#include "sysdef.h"

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
    struct FileSystem *fs;
};

struct FileSystemOperation
{
    s64int (*root)(struct FileSystem *fs);
    s64int (*open)(struct FileSystem *fs, s64int id, s64int *openId);
    s64int (*close)(struct FileSystem *fs, s64int id);
    s64int (*read)(struct FileSystem *fs, s64int id, u64int offset, u64int size, char *buffer);
    s64int (*write)(struct FileSystem *fs, s64int id, u64int offset, u64int size, char *buffer);
    s64int (*stat)(struct FileSystem *fs, s64int id, struct VNodeInfo *ni);
    s64int (*ioctl)(struct FileSystem *fs, s64int request, void *data);
    s64int (*readdir)(struct FileSystem *fs, s64int id, u64int bufSize, char *buf);
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
    void *data;
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
s64int vfsRead(struct VNode *node, u64int offset, u64int size, void *buffer);
s64int vfsWrite(struct VNode *node, u64int offset, u64int size, char *buffer);
s64int vfsState(const char *path, struct VNodeInfo *ni);

s64int vfsCreateObject(const char *name, s64int id);
s64int vfsCreateDirectory(const char *name);
s64int vfsRemoveDirectory(const char *name);
s64int vfsReadDirectory(struct VNode *vnd, u64int bufSize, char *buf);

void initVFS();

#endif /* VFS_H */
