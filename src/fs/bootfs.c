#include "bootfs.h"
#include "vfs.h"
#include "util.h"
#include "kmm.h"

s64int bootfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
s64int bootfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs);

s64int bootfsOpen(struct FileSystem *fs, const char *path, u64int flags, struct VNode **ret);
s64int bootfsClose(struct FileSystem *fs, struct VNode *);
s64int bootfsRead(struct FileSystem *fs, struct VNode *node, u64int offset, u64int size, char *buffer);
s64int bootfsReaddir(struct FileSystem *fs, struct VNode *node, u64int startIndex, u64int bufSize, char *buf);
s64int bootfsStat(struct FileSystem *fs, const char *path, struct VNodeInfo *info);

struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checkSum[8];
    char link;
    char linkName[100];
} __attribute__((packed));

struct BootFSData {
    u64int num;
    char rootDirName[100];
    char *buf;
};

struct FileSystemDriverOperation driverOps = {
    .mount = bootfsMount,
    .unmount = bootfsUnmount,
};

struct FileSystemOperation fsOps = {
    .open = bootfsOpen,
    .close = bootfsClose,
    .read = bootfsRead,
    .write = 0,
    .mkdir = 0,
    .rmdir = 0,
    .readdir = bootfsReaddir,
    .stat = bootfsStat,
} ;

struct FileSystemDriver bootfsDriver = {
    .name = "bootfs",
    .op = &driverOps,
};

s64int isEmptyEntry(char *buf)
{
    u32int i;
    for (i=0; i<512; i++)
        if (buf[i])
            return 0;
    return 1;
}

s64int bootfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data)
{
    struct BootFSData *fsData;
    struct TarHeader *hd;
    u64int i;

    if (source && strlen(source)) {
        return -1;
    }

    *fs = (struct FileSystem*)kMalloc(sizeof(struct FileSystem));
    memset(*fs, 0, sizeof(struct FileSystem));
    (*fs)->driver = &bootfsDriver;
    (*fs)->op = &fsOps;
    fsData = (struct BootFSData *)kMalloc(sizeof(struct BootFSData));
    memset(fsData, 0, sizeof(struct BootFSData));
    fsData->buf = (char *)data;
    i = 0;
    while (1) {
        if (isEmptyEntry(fsData->buf+512*i))
            break;
        hd = (struct TarHeader *)(fsData->buf+512*i);
        if (i==0)
            strcpy(fsData->rootDirName,hd->name);
        i++;
    }
    fsData->num = i;
    if (fsData->num == 0)
        strcpy(fsData->rootDirName,"/");
    
    (*fs)->data = fsData;
    return 0;
}

s64int bootfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs)
{
    if (fs->driver != &bootfsDriver)
        return -1;
    kFree(fs->data);
    return 0;
}

s64int bootfsOpen(struct FileSystem *fs, const char *path, u64int flags, struct VNode **pnode)
{
    char *fullPath;
    struct BootFSData *fsData;
    struct TarHeader *h;
    u64int i;
    s64int ret;

    fsData = (struct BootFSData*)fs->data;
    fullPath = (char *)kMalloc(strlen(path)+101);
    strcpy(fullPath, fsData->rootDirName);
    strcpy(fullPath+strlen(fullPath)-1, path);

    *pnode = 0;
    ret = -1;
    for (i=0; i<fsData->num; i++) {
        h = (struct TarHeader*)(fsData->buf+512*i);
        if (strcmp(h->name,fullPath)==0) {
            *pnode = (struct VNode*)kMalloc(sizeof(struct VNode));
            memset(*pnode,0,sizeof(struct VNode));
            (*pnode)->id = i;
            (*pnode)->fs = fs;
            ret = 0;
            break;
        }
    }

    return ret;
}

s64int bootfsClose(struct FileSystem *fs, struct VNode *node)
{
    kFree(node);

    return 0;
}

s64int bootfsRead(struct FileSystem *fs, struct VNode *node, u64int offset, u64int size, char *buffer)
{

    return 0;
}

s64int bootfsReaddir(struct FileSystem *fs, struct VNode *node, u64int startIndex, u64int bufSize, char *buf)
{

    return 0;
}

s64int bootfsStat(struct FileSystem *fs, const char *path, struct VNodeInfo *info)
{
    return 0;
}


