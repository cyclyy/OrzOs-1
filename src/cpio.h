#ifndef CPIO_H
#define CPIO_H 

#include "common.h"

#define CPIO_MAGIC   070707
#define CPIO_FT_MASK 0170000
#define CPIO_FT_FILE 0100000
#define CPIO_FT_DIR  0040000

typedef struct header_cpio_struct {
    u16int c_magic;
    u16int c_dev;
    u16int c_ino;
    u16int c_mode;
    u16int c_uid;
    u16int c_gid;
    u16int c_nlink;
    u16int c_rdev;
    u16int c_mtime[2];
    u16int c_namesize;
    u16int c_filesize[2];
} header_cpio_t;

s32int extract_cpio(u8int *buf, u32int size);

#endif /* CPIO_H */
