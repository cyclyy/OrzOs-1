#include "vfs.h"
#include "kmm.h"
#include "util.h"
#include "fs/bootfs.h"
#include "fs/devfs.h"

struct MountPoint {
    char *name;
    struct FileSystem *fs;
    struct MountPoint *next, *prev;
};

struct MountPoint *mountPoints = 0;

struct FileSystemDriver *fsDrivers = 0;

struct VNode *rootNode = 0;

s64int vfsLookup(const char *path, struct VNode *node);

struct FileSystemDriver *getFSDriver(const char *name)
{
    struct FileSystemDriver *d;
    d = fsDrivers;
    while (d) {
        if (strcmp(d->name, name)==0) 
            return d;
        d = d->next;
    }
    return 0;
}

struct MountPoint *getMountPoint(const char *name)
{
    struct MountPoint *mt;
    mt = mountPoints;
    while (mt) {
        if (strcmp(mt->name, name)==0)
            return mt;
        mt = mt->next;
    }
    return 0;
}

s64int registerFileSystemDriver(struct FileSystemDriver *driver)
{
    if (getFSDriver(driver->name)) 
        return -1;
    driver->next = fsDrivers;
    if (fsDrivers)
        fsDrivers->prev = driver;
    fsDrivers = driver;

    return 0;
}

s64int unregisterFileSystemDriver(const char *name)
{
    struct FileSystemDriver *driver;

    driver = getFSDriver(name);

    if (!driver)
        return -1;

    if (driver->prev)
        driver->prev->next = driver->next;
    if (driver->next)
        driver->next->prev = driver->prev;

    if (fsDrivers == driver)
        fsDrivers = driver->next;

    return 0;
}

s64int vfsMount(const char *dest, const char *source, const char *fsType, u64int flags, void *data)
{
    struct FileSystemDriver *driver;
    struct FileSystem *fs;
    struct MountPoint *mt;
    s64int ret;

    driver = getFSDriver(fsType);

    if (!driver || !driver->op)
        return -1;

    if (!driver->op->mount)
        return -1;

    if (getMountPoint(dest))
        return -1;

    ret = driver->op->mount(driver, &fs, source, flags, data);
    if (ret != 0)
        return ret;

    mt = (struct MountPoint*)kMalloc(sizeof(struct MountPoint));
    memset(mt, 0, sizeof(struct MountPoint));
    mt->name = (char*)kMalloc(strlen(dest)+1);
    strcpy(mt->name,dest);
    mt->fs = fs;
    mt->next = mountPoints;
    if (mountPoints)
        mountPoints->prev = mt;
    mountPoints = mt;

    return 0;
}

s64int vfsUnmount(const char *path)
{
    struct MountPoint *mt;
    mt = getMountPoint(path);
    if (!mt) 
        return -1;

    if (mt->prev)
        mt->prev->next = mt->next;
    if (mt->next)
        mt->next->prev = mt->prev;

    if (mountPoints == mt)
        mountPoints = mt->next;

    mt->fs->driver->op->unmount(mt->fs->driver, mt->fs);
    kFree(mt->name);
    kFree(mt);
    
    return 0;
}

struct MountPoint *findMountPointPrefix(const char *path)
{
    struct MountPoint *mt = 0;
    char buf[MAX_NAME_LEN+1];
    char *s;

    strcpy(buf,path);
    s = strchr(buf,':');
    if (!s)
        return 0;
    buf[s - buf] = 0;
    mt = mountPoints;
    while (mt) {
        if (strcmp(buf,mt->name)==0)
            return mt;
        mt = mt->next;
    }

    return 0;
}

s64int parsePath(const char *path, struct MountPoint **mt, char *remainPath)
{
    *mt = findMountPointPrefix(path);
    if (!*mt)
        return -1;

    if (path[strlen((*mt)->name) + 1])
        strcpy(remainPath, path+strlen((*mt)->name)+1);
    else
        strcpy(remainPath,"/");

    /*
    DBG((*mt)->name);
    DBG(path + strlen((*mt)->name) +1);
    */

    return 0;
}

s64int extractPath(char *path, char *name)
{
    char *s;
    u64int i, len;

    if ((strlen(path)==0) || (strcmp(path,"/")==0))
        return 0;
    s = strchr(path+1,'/');
    if (s) {
        len = strlen(s);
        for (i=0; i<len; i++)
            path[i] = s[i];
        path[len] = 0;
    } else {
        strcpy(name, path+1);
        strcpy(path, "/");
    }

    return 1;
}

s64int vfsOpen(const char *path, u64int flags, struct VNode *node)
{
    struct MountPoint *mt;
    char *remainPath, name[MAX_NAME_LEN];
    s64int ret, id;

    ret = 0;
    remainPath = (char*)kMalloc(strlen(path)+1);
    parsePath(path, &mt, remainPath);
    //DBG(remainPath);

    if (mt) {
        node->fs = mt->fs;
        id = mt->fs->op->root(mt->fs);
        while (1) {
            ret = extractPath(remainPath, name);
            if (ret <= 0)
                break;
            id = mt->fs->op->finddir(mt->fs, id, name);
            if (id <= 0) {
                ret = -1;
                break;
            }
        }
        if (ret==0) {
            ret = mt->fs->op->open(mt->fs, id, &node->id);
        }
    } else
        ret = -1;
            
    kFree(remainPath);

    return ret;
}

s64int vfsClose(struct VNode *node)
{
    if (node->fs && node->fs->op->close)
        return node->fs->op->close(node->fs, node->id);
    else
        return -1;
}

s64int vfsRead(struct VNode *node, u64int offset, u64int size, void *buffer)
{
    if (node->fs && node->fs->op->read)
        return node->fs->op->read(node->fs, node->id, offset, size, buffer);
    else
        return -1;
}

s64int vfsWrite(struct VNode *node, u64int offset, u64int size, char *buffer)
{
    if (node->fs && node->fs->op->write)
        return node->fs->op->write(node->fs, node->id, offset, size, buffer);
    else
        return -1;
}

s64int vfsState(const char *path, struct VNodeInfo *ni)
{
    struct VNode node;
    s64int ret;

    ret = vfsLookup(path, &node);
    if (ret == 0) {
        if (node.fs && node.fs->op->stat)
            ret = node.fs->op->stat(node.fs, node.id, ni);
        else
            ret = -1;
    }

    return ret;
}

s64int vfsIoControl(struct VNode *node, s64int request, void *data, u64int size)
{
    if (node->fs && node->fs->op->ioctl)
        return node->fs->op->ioctl(node->fs, node->id, request, data, size);
    else
        return -1;
}

s64int vfsLookup(const char *path, struct VNode *node)
{
    struct MountPoint *mt;
    char *remainPath, name[MAX_NAME_LEN];
    s64int ret, id;

    ret = 0;
    remainPath = (char*)kMalloc(strlen(path)+1);
    parsePath(path, &mt, remainPath);
    //DBG(remainPath);

    if (mt) {
        node->fs = mt->fs;
        id = mt->fs->op->root(mt->fs);
        while (1) {
            ret = extractPath(remainPath, name);
            if (ret <= 0)
                break;
            id = mt->fs->op->finddir(mt->fs, id, name);
            if (id <= 0) {
                ret = -1;
                break;
            }
        }
        if (ret==0) {
            node->id = id;
        }
    } else
        ret = -1;
            
    kFree(remainPath);

    return ret;
}

s64int vfsCreateObject(const char *path, s64int objid)
{
    char *dirPath, baseName[MAX_NAME_LEN];
    struct VNode node;
    s64int ret;

    ret = 0;
    dirPath = (char*)kMalloc(strlen(path)+1);
    dirname(dirPath, path);
    basename(baseName, path);
    ret = vfsLookup(dirPath, &node);
    if (ret == 0) {
        if (node.fs && node.fs->op->mkobj)
            ret = node.fs->op->mkobj(node.fs, node.id, baseName, objid);
        else
            ret = -1;
    }
    kFree(dirPath);

    return ret;
}

s64int vfsCreateDirectory(const char *path)
{
    char *dirPath, baseName[MAX_NAME_LEN];
    struct VNode node;
    s64int ret;

    ret = 0;
    dirPath = (char*)kMalloc(strlen(path)+1);
    dirname(dirPath, path);
    basename(baseName, path);
    ret = vfsLookup(dirPath, &node);
    if (ret == 0) {
        if (node.fs && node.fs->op->mkdir)
            ret = node.fs->op->mkdir(node.fs, node.id, baseName);
        else
            ret = -1;
    }
    kFree(dirPath);

    return ret;
}

s64int vfsRemoveDirectory(const char *path)
{
    char *dirPath, baseName[MAX_NAME_LEN];
    struct VNode node;
    s64int ret;

    ret = 0;
    dirPath = (char*)kMalloc(strlen(path)+1);
    dirname(dirPath, path);
    basename(baseName, path);
    ret = vfsLookup(dirPath, &node);
    if (ret == 0) {
        if (node.fs && node.fs->op->mkdir)
            ret = node.fs->op->rmdir(node.fs, node.id, baseName);
        else
            ret = -1;
    }
    kFree(dirPath);

    return ret;
}

s64int vfsReadDirectory(struct VNode *node, u64int bufSize, char *buffer)
{
    if (node->fs && node->fs->op->readdir)
        return node->fs->op->readdir(node->fs, node->id, bufSize, buffer);
    else
        return -1;
}

void initVFS()
{
    registerFileSystemDriver(&bootfsDriver);
    registerFileSystemDriver(&devfsDriver);
}
