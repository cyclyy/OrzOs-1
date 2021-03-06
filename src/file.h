#ifndef FILE_H
#define FILE_H 

#include "common.h"

#define FD_INVALID -1

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   4
#define O_EXEC   8

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// represent a opened file

struct file_operations;
struct vnode_struct;
typedef struct file_struct file_t;
    
struct file_struct {
    u32int offset;
    u32int flags;
    struct vnode_struct *vnode;
    struct file_operations *f_ops;

    void *priv;
};

typedef struct dir_struct {
    u32int offset;
    u32int nsubs;
    struct vnode_struct **subnodes;
} dir_t;

typedef struct fd_struct {
    file_t *f;
    dir_t *d;
} fd_t;

typedef struct dentry_struct {
    char name[MAX_NAME_LEN];
    u32int flags;
} dentry_t;

typedef struct {
    file_t *file;
    s32int fd;
    u32int offset;
    u32int size;
} file_mapping_t;

dir_t*  dir_open    (char *path, u32int flags);
s32int  dir_close   (dir_t *d);

file_t* file_open   (char *path, u32int flags);
s32int  file_read   (file_t *f,  void *buf, u32int sz);
s32int  file_write  (file_t *f,  void *buf, u32int sz);
s32int  file_close  (file_t *f);
s32int  file_lseek  (file_t *f, s32int offset, u32int whence);

file_mapping_t *clone_file_mapping(file_mapping_t *f_map);
file_t *clone_file(file_t *f);

s32int sys_fdopen(char *name, u32int flags);
s32int sys_fdclose(s32int fd);
s32int sys_fdread(s32int fd, void *buf, u32int size);
s32int sys_fdwrite(s32int fd, void *buf, u32int size);
s32int sys_fdlseek(s32int fd, s32int offset, u32int whence);

s32int sys_getdents(u32int fd, u8int *buf, u32int size);

#endif /* FILE_H */
