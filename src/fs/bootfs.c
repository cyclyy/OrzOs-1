#include "bootfs.h"
#include "vfs.h"
#include "util.h"
#include "kmm.h"

s64int bootfsMount(struct FileSystemDriver *driver, struct FileSystem **fs, const char *source, u64int flags, void *data);
s64int bootfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs);

s64int bootfsRoot(struct FileSystem *fs);
s64int bootfsOpen(struct FileSystem *fs, s64int id, struct VNode *node);
s64int bootfsClose(struct VNode *node);
s64int bootfsRead(struct VNode *node, u64int size, char *buffer);
s64int bootfsSeek(struct VNode *node, s64int offset, s64int pos);
s64int bootfsReaddir(struct VNode *node, u64int size, char *buf);
s64int bootfsFinddir(struct FileSystem *fs, s64int id, const char *name);
s64int bootfsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info);

#define TAR_LINK_DIR    '5'
#define TAR_LINK_FILE   '0'

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
    struct TarHeader **headerList;
    char *buf;
};

static struct FileSystemDriverOperation driverOps = {
    .mount = bootfsMount,
    .unmount = bootfsUnmount,
};

static struct FileSystemOperation fsOps = {
    .root = bootfsRoot,
    .open = bootfsOpen,
    .close = bootfsClose,
    .read = bootfsRead,
    .write = 0,
    .readdir = bootfsReaddir,
    .finddir = bootfsFinddir,
    .stat = bootfsStat,
    .seek = bootfsSeek,
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
    u64int i, n;

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
    i = n = 0;
    while (1) {
        if (isEmptyEntry(fsData->buf+i))
            break;
        hd = (struct TarHeader *)(fsData->buf+i);
        if (i==0)
            strcpy(fsData->rootDirName,hd->name);
        i += 512;
        i += (strtoul(hd->size, 0, 8) + 511) & (~0 - 511);
        n++;
    }
    fsData->num = n;
    if (n == 0)
        strcpy(fsData->rootDirName,"/");
    if (n) {
        fsData->headerList = (struct TarHeader **)kMalloc(sizeof(struct TarHeader*)*n);
        i = n = 0;
        while (n < fsData->num) {
            hd = (struct TarHeader *)(fsData->buf+i);
            fsData->headerList[n] = hd;
            i += 512;
            i += (strtoul(hd->size, 0, 8) + 511) & (~0 - 511);
            n++;
        }
    }
    
    (*fs)->priv = fsData;
    return 0;
}

s64int bootfsUnmount(struct FileSystemDriver *driver, struct FileSystem *fs)
{
    struct BootFSData *fsData;

    fsData = (struct BootFSData*)fs->priv;
    kFree(fsData->headerList);
    kFree(fsData);
    return 0;
}

s64int bootfsRoot(struct FileSystem *fs)
{
    return 1;
}

s64int bootfsOpen(struct FileSystem *fs, s64int id, struct VNode *node)
{
    node->fs = fs;
    node->id = id;
    node->offset = 0;
    return 0;
}

s64int bootfsClose(struct VNode *node)
{
    return 0;
}

s64int bootfsRead(struct VNode *node, u64int size, char *buffer)
{
    struct TarHeader *hd;
    struct BootFSData *fsData;
    s64int ret, fileSize;

    fsData = (struct BootFSData*)node->fs->priv;
    hd = fsData->headerList[node->id-1];
    if (hd->link != TAR_LINK_FILE)
        return -1;
    fileSize = strtoul(hd->size, 0, 8);
    if ((node->offset>=0) && (node->offset<=fileSize)) {
        ret = MIN(size, fileSize - node->offset);
        memcpy(buffer, ((char*)hd) + 512 + node->offset, ret);
        node->offset += ret;
    } else 
        ret = -1;

    return ret;
}

s64int bootfsReaddir(struct VNode *node, u64int bufSize, char *buf)
{
    struct BootFSData *fsData;
    struct TarHeader *hd, *hd1;
    struct DirectoryRecord *drec;
    s64int id, ret, i, off, len;
    u64int pos, j, n, ok, dsize;
    char *s, s1[MAX_NAME_LEN];

    fsData = (struct BootFSData*)node->fs->priv;

    id = node->id;
    if ((id <= 0) || (id > fsData->num))
        return -1;

    id--;
    hd = fsData->headerList[id];
    if (hd->link != TAR_LINK_DIR)
        return -1;

    pos = j = off = 0;
    s = strrchr(hd->name, '/');
    if (s && (strlen(s)==1)) {
        ret = 0;
        len = strlen(hd->name);
        for (i=id+1; i<fsData->num; i++) {
            hd1 = fsData->headerList[i];
            if (strstr(hd1->name,hd->name) == hd1->name) {
                strcpy(s1, hd1->name + len);
                s = strchr(s1,'/');
                if (s) 
                    s1[s-s1] = 0;
                j = n = 0;
                ok = 1;
                while (n < ret) {
                    drec = (struct DirectoryRecord *)(buf + j);
                    if (strcmp(drec->buffer, s1) == 0) {
                        ok = 0;
                        break;
                    }
                    j += drec->size;
                    n++;
                }
                if (ok) {
                    drec = (struct DirectoryRecord *)(buf + pos);
                    dsize = sizeof(struct DirectoryRecord) + strlen(s1);
                    if (pos + dsize <= bufSize) {
                        if (off == node->offset) {
                            drec->size = dsize;
                            strcpy(drec->buffer, s1);
                            pos += dsize;
                            ret++;
                            node->offset++;
                        }
                        off++;
                    } else 
                        break;
                }
            }
        }
    } else
        ret = -1;


    return ret;
}

s64int bootfsFinddir(struct FileSystem *fs, s64int id, const char *name)
{
    struct BootFSData *fsData;
    struct TarHeader *hd;
    s64int ret, i;
    char *s;

    fsData = (struct BootFSData*)fs->priv;
    id--;
    hd = fsData->headerList[id];
    if (hd->link != TAR_LINK_DIR)
        return -1;
    s = (char *)kMalloc(strlen(hd->name) + strlen(name) + 2);
    strcpy(s, hd->name);
    strcpy(s+strlen(s), name);

    ret = 0;
    for (i=id+1; i<fsData->num; i++) {
        hd = fsData->headerList[i];
        if (strcmp(hd->name, s)==0) {
            ret = i+1;
            break;
        }
    }

    return ret;
}

s64int bootfsStat(struct FileSystem *fs, s64int id, struct VNodeInfo *info)
{
    return 0;
}

s64int bootfsSeek(struct VNode *node, s64int offset, s64int pos)
{
    struct TarHeader *hd;
    struct BootFSData *fsData;
    s64int fileSize, newOffset;

    fsData = (struct BootFSData*)node->fs->priv;
    hd = fsData->headerList[node->id-1];
    if (hd->link != TAR_LINK_FILE) {
        return -1;
    }
    fileSize = strtoul(hd->size, 0, 8);
    switch (pos) {
    case SEEK_SET:
        newOffset = offset;
        break;
    case SEEK_CUR:
        newOffset = node->offset + offset;
        break;
    case SEEK_END:
        newOffset = fileSize + offset;
        break;
    default:
        return -1;
    }
    if ((newOffset>=0) && (newOffset<=fileSize)) {
        node->offset = newOffset;
        return 0;
    } else {
        return -1;
    }
}

