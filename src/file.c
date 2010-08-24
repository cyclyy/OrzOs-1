#include "file.h"
#include "kheap.h"
#include "task.h"
#include "vfs.h"

file_t* file_open(char *path, u32int flags)
{
    file_t *f = (file_t*)kmalloc(sizeof(file_t));
    memset(f,0,sizeof(file_t));
    strcpy(f->path, path);
    f->vnode = vfs_lookup(vfs_root, path);
    if (f->vnode) {
        vfs_open(f->vnode);
        return f;
    } else {
        kfree(f->path);
        kfree(f);
        return 0;
    }
}

s32int file_read(file_t *f, void *buf, u32int sz)
{
    u32int n = vfs_read(f->vnode, f->offset, sz, buf);
    f->offset += n;

    return n;
}

s32int file_write(file_t *f, void *buf, u32int sz)
{
    u32int n = vfs_write(f->vnode, f->offset, sz, buf);
    f->offset += n;

    return n;
}

void file_close(file_t *f)
{
    vfs_close(f->vnode);
    kfree(f->path);
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


s32int fdopen(char *name, u32int flags)
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
        current_task->fd[i] = file_open(name, flags);

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

