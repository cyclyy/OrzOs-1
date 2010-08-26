#include "file.h"
#include "kheap.h"
#include "task.h"
#include "vfs.h"
#include "dev.h"

file_t* file_open(char *path, u32int flags)
{
    if (!path)
        return 0;

    vnode_t *node = vfs_lookup(vfs_root, path);
    if (!node)
        return 0;

    file_t *f = (file_t*)kmalloc(sizeof(file_t));
    memset(f,0,sizeof(file_t));
    f->vnode = node;
    if (node->flags & VFS_DEV_FILE) {
        dev_t *dev  = find_dev(node->dev_id);
        if (dev) {
            f->open = dev->open;
            f->read = dev->read;
            f->write= dev->write;
            f->close= dev->close;
            f->priv = dev;
        }
    } else if (node->flags & VFS_FILE) {
        f->open     = node->open;
        f->read     = node->read;
        f->write    = node->write;
        f->close    = node->close;
    }
    if (f->open)
        f->open(f);
    return f;
}

s32int file_read(file_t *f, void *buf, u32int sz)
{
    u32int n = f->read(f, f->offset, sz, buf);
    f->offset += n;

    return n;
}

s32int file_write(file_t *f, void *buf, u32int sz)
{
    u32int n = f->write(f, f->offset, sz, buf);
    f->offset += n;

    return n;
}

void file_close(file_t *f)
{
    f->close(f);
    kfree(f);
}

s32int file_lseek(file_t *f, s32int offset, u32int whence)
{
    switch (whence) {
        case SEEK_SET:
            f->offset = offset;
            break;
        case SEEK_CUR:
            f->offset += offset;
            break;
        case SEEK_END:
            f->offset = f->vnode->length + offset;
            break;
        default:
            // return an error code
            ;
    }

    return f->offset;
}

file_mapping_t *clone_file_mapping(file_mapping_t *f_map)
{
    if (!f_map)
        return 0;

    file_mapping_t *ret = (file_mapping_t*)kmalloc(sizeof(file_mapping_t));
    memcpy(ret, f_map, sizeof(file_mapping_t));

    ret->file = clone_file(f_map->file);

    return ret;
}

file_t *clone_file(file_t *f)
{
    if (!f)
        return 0;

    file_t *ret = (file_t*)kmalloc(sizeof(file_t));
    memcpy(ret, f, sizeof(file_t));

    return ret;
}


s32int fdopen(char *path, u32int flags)
{
    u32int i; 
    s32int find; 
    find = -1;
    for (i=0; i<MAX_TASK_FDS; i++) {
        if (current_task->fd[i] == 0) {
            find = i;
            break;
        }
    }

    if (find >= 0) {
        current_task->fd[i] = file_open(path, flags);

        if (current_task->fd[i]) {
            return find;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

s32int fdclose(s32int fd)
{
    file_t *f = current_task->fd[fd];

    if (f) {
        file_close(f);
        return 0;
    }
    return -1;
}

s32int fdread(s32int fd, void *buf, u32int size)
{
    file_t *f = current_task->fd[fd];

    if (f) {
        return file_read(f, buf, size);
    }
    return -1;
}

s32int fdwrite(s32int fd, void *buf, u32int size)
{
    file_t *f = current_task->fd[fd];

    if (f) {
        return file_write(f, buf, size);
    }
    return -1;
}

s32int fdlseek(s32int fd, s32int offset, u32int whence)
{
    file_t *f = current_task->fd[fd];

    if (f) {
        return file_lseek(f, offset, whence);
    }
    return -1;
}

s32int sys_fdopen(char *path, u32int flags)
{
    char *_path = (char*)kmalloc(MAX_PATH_LEN);
    u32int n = copy_from_user(_path, path, MAX_PATH_LEN);
    u32int ret = fdopen(_path, flags);
    kfree(_path);
    return ret;
}

s32int sys_fdclose(s32int fd)
{
    return fdclose(fd);
}

s32int sys_fdread(s32int fd, void *buf, u32int size)
{
    char *kbuf = (char*)kmalloc(PAGE_SIZE);
    u32int n = 0;
    u32int len;
    u32int copied;
    u32int ret = 0;

    do {
        len = fdread(fd,kbuf,MIN(PAGE_SIZE,size));
        if (len<0) {
            ret = len;
            break;
        }

        copied = copy_to_user(buf+n, kbuf, len);

        if (copied < len) {
            ret = -EFAULT;
            break;
        }

        n += copied;

        size -= len;
    } while (size);

    if (ret == 0)
        ret = n;

    kfree(kbuf);

    return ret;
}

s32int sys_fdwrite(s32int fd, void *buf, u32int size)
{
    char *kbuf = (char*)kmalloc(PAGE_SIZE);
    u32int n = 0;
    u32int len;
    u32int copied;
    u32int ret = 0;

    do {
        copied = copy_from_user(kbuf, buf+n, MIN(PAGE_SIZE,size));

        if (copied < MIN(PAGE_SIZE,size)) {
            ret = -EFAULT;
            break;
        }

        len = fdwrite(fd,kbuf,copied);

        if (len<0) {
            ret = len;
            break;
        }


        n += len;

        size -= len;
    } while (size);

    if (ret == 0)
        ret = n;

    kfree(kbuf);

    return ret;
    return fdwrite(fd,buf,size);
}

s32int sys_fdlseek(s32int fd, s32int offset, u32int whence)
{
    return fdlseek(fd, offset, whence);
}

