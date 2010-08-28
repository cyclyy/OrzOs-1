#include "file.h"
#include "kheap.h"
#include "task.h"
#include "vfs.h"
#include "dev.h"

static u32int fd_valid(u32int fd)
{
    if ((fd<0) || (fd>=MAX_TASK_FDS))
        return 0;

    if (!current_task->fd[fd])
        return 0;

    return 1;
}

dir_t*  dir_open    (char *path, u32int flags)
{
    if (!path)
        return 0;
    vnode_t *node = vfs_lookup(path);
    if (!node || !(node->flags & VFS_DIRECTORY))
        return 0;
    dir_t *d = (dir_t*)kmalloc(sizeof(dir_t));
    memset(d,0,sizeof(dir_t));
    d->nsubs = vfs_subnodes(path,&d->subnodes);
    if (d->nsubs<0) {
        kfree(d);
        return 0;
    }
    return d;
}

s32int dir_close(dir_t *d)
{
    if (!d)
        return -EFAULT;

    kfree(d->subnodes);
    kfree(d);

    return 0;
}

file_t* file_open(char *path, u32int flags)
{
    if (!path)
        return 0;

    vnode_t *node = vfs_lookup(path);
    if (!node)
        return 0;

    file_t *f = (file_t*)kmalloc(sizeof(file_t));
    memset(f,0,sizeof(file_t));
    f->vnode = node;
    if (node->flags & VFS_DEV_FILE) {
        dev_t *dev  = find_dev(node->dev_id);
        if (dev) {
            f->f_ops = dev->f_ops;
            f->priv = dev;
        }
    } else if (node->flags & VFS_FILE) {
        f->f_ops = node->f_ops;
    }
    if (f->f_ops && f->f_ops->open)
        f->f_ops->open(f);
    return f;
}

s32int file_read(file_t *f, void *buf, u32int sz)
{
    if (f && f->f_ops && f->f_ops->read) {
        u32int n = f->f_ops->read(f, f->offset, sz, buf);
        f->offset += n;
        return n;
    } else
        return 0;

}

s32int file_write(file_t *f, void *buf, u32int sz)
{
    if (!f)
        return -EFAULT;
    if (f && f->f_ops && f->f_ops->write) {
        u32int n = f->f_ops->write(f, f->offset, sz, buf);
        f->offset += n;

        return n;
    } else 
        return 0;
}

s32int file_close(file_t *f)
{
    if (!f)
        return -EFAULT;
    if (f && f->f_ops && f->f_ops->write) {
        s32int ret = f->f_ops->close(f);
        kfree(f);
        return ret;
    }

    return -EFAULT;
}

s32int file_lseek(file_t *f, s32int offset, u32int whence)
{
    if (!f)
        return 0;
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

s32int sys_fdopen(char *path, u32int flags)
{
    char *kpath = (char*)kmalloc(MAX_PATH_LEN);
    memset(kpath,0,MAX_PATH_LEN);
    s32int i, find, ret; 
    u32int n = copy_from_user(kpath, path, MAX_PATH_LEN);
    
    if (n==0) {
        ret = -EFAULT;
        goto cleanup;
    }

    find = -1;
    for (i=0; i<MAX_TASK_FDS; i++) {
        if (current_task->fd[i] == 0) {
            find = i;
            break;
        }
    }

    if (find >= 0) {
        char *abs_path = vfs_abs_path(kpath);

        vnode_t *node = vfs_lookup(abs_path);

        if (!node) {
            ret = -ENOENT;
            goto cleanup;
        }

        current_task->fd[i] = (fd_t*)kmalloc(sizeof(fd_t));
        memset(current_task->fd[i],0,sizeof(fd_t));

        if (node->flags & VFS_DIRECTORY) {
            current_task->fd[i]->d = dir_open(abs_path, flags);
        } else {
            current_task->fd[i]->f = file_open(abs_path, flags);
        }
        ret = find;
        kfree(abs_path);
    } else {
        ret = -EFAULT;
    }

cleanup:
    kfree(kpath);
    return ret;
}

s32int sys_fdclose(s32int fd)
{
    if (!fd_valid(fd))
        return -EINVAL;

    s32int ret;
    fd_t *fd_st = current_task->fd[fd];
    if (fd_st->f) {
        ret = file_close(fd_st->f);
    } else if (fd_st->d) {
        ret = dir_close(fd_st->d);
    }

    kfree(fd_st);
    current_task->fd[fd] = 0;
    return ret;
}

s32int sys_fdread(s32int fd, void *buf, u32int size)
{
    if (!fd_valid(fd))
        return -EINVAL;

    file_t *f = current_task->fd[fd]->f;

    char *kbuf = (char*)kmalloc(PAGE_SIZE);
    u32int n = 0;
    u32int len;
    u32int copied;
    u32int ret = 0;

    do {
        len = file_read(f,kbuf,MIN(PAGE_SIZE,size));
        if (len<=0) {
            ret = len;
            break;
        }

        copied = copy_to_user(buf+n, kbuf, len);

        if (copied < len) {
            ret = -EFAULT;
            break;
        }

        n += copied;

        if (len<MIN(PAGE_SIZE,size))
            break;
        size -= len;
    } while (size);

    if (ret == 0)
        ret = n;

    kfree(kbuf);

    return ret;
}

s32int sys_fdwrite(s32int fd, void *buf, u32int size)
{
    if (!fd_valid(fd))
        return -EINVAL;

    file_t *f = current_task->fd[fd]->f;

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

        len = file_write(f, kbuf, copied);

        if (len<=0) {
            ret = len;
            break;
        }


        n += len;

        if (len<copied)
            break;

        size -= len;
    } while (size);

    if (ret == 0)
        ret = n;

    kfree(kbuf);

    return ret;
}

s32int sys_fdlseek(s32int fd, s32int offset, u32int whence)
{
    if (!fd_valid(fd))
        return -EINVAL;

    file_t *f = current_task->fd[fd]->f;

    return file_lseek(f, offset, whence);
}

s32int          sys_getdents(u32int fd, u8int *buf, u32int size)
{
    if (!fd_valid(fd)) 
        return -EBADF;
    if (size < sizeof(dentry_t)) 
        return -EINVAL;
    dir_t *d = current_task->fd[fd]->d;
    if (!d)
        return -ENOTDIR;

    if (d->offset>=d->nsubs)
        return 0;
    else {
        u32int n, len, i;
        u8int *kbuf;
        dentry_t *dentry;
        n = MIN(d->nsubs - d->offset,size/sizeof(dentry_t));
        len = n * sizeof(dentry_t);
        kbuf = (u8int*)kmalloc(len);
        memset(kbuf,0,len);
        dentry = (dentry_t*)kbuf;
        for (i=0; i<n; i++) {
            strcpy(dentry[i].name, d->subnodes[d->offset + i]->name);
            dentry[i].flags = d->subnodes[d->offset  + i]->flags;
        }

        u32int copied, ret;
        copied = copy_to_user(buf,kbuf,len);
        if (copied<len)
            ret = -EFAULT;
        else {
            d->offset += n;
            ret = len;
        }

        kfree(kbuf);

        return ret;
    }
}

