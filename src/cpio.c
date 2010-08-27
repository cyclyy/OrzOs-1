#include "cpio.h"
#include "file.h"
#include "vfs.h"
#include "screen.h"

s32int extract_cpio(u8int *buf, u32int size)
{
    header_cpio_t *header = 0;
    u32int n = 0, filesize;
    char *path, *abs_path;
    while (n<size) {
        header = (header_cpio_t*)(buf+n);
        n += sizeof(header_cpio_t);
        path = (char*)(buf + n);
        filesize = (header->c_filesize[0]<<16) + header->c_filesize[1];
        n += (header->c_namesize+1)&(~1);
        if (header->c_magic != CPIO_MAGIC) {
            return -1;
        }
        // tailer header
        if (header->c_mode == 0)
            break;
        switch (header->c_mode & CPIO_FT_MASK) {
            case CPIO_FT_FILE:
                abs_path = vfs_abs_path(path);
                printk("File %s Size %d\n", abs_path,filesize);
                if (abs_path) {
                    vfs_create(abs_path,0);
                    file_t *f = file_open(abs_path,0);
                    file_write(f,buf+n,filesize);
                    file_close(f);
                }
                kfree(abs_path);
                break;
            case CPIO_FT_DIR:
                abs_path = vfs_abs_path(path);
                printk("Dir %s\n", abs_path);
                // skip root dir
                if (abs_path && (strcmp(abs_path,"/")!=0))
                    vfs_mkdir(abs_path,0);
                kfree(abs_path);
                break;
            default:
                ;
        };
        n += (filesize+1)&(~1);
    }
    return 0;
}

