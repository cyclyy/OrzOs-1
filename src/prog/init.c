#include "syscall.h"
#include "file.h"

#define VMA_ANON      1     /* memory mapping, no back-file*/
#define VMA_READ      2  
#define VMA_WRITE     4 
#define VMA_EXEC      8
#define VMA_STACK     16 
#define VMA_PRIVATE   32 
#define VMA_FILE      64 
#define VMA_SHARED    128

void puts(char *s)
{
    while (*s)
        syscall_putch(*s++);
}

void putn(u32int n)
{
    u32int base = 1000000;
    u32int i = 0;
    
    if (n==0)
        syscall_putch('0');

    while (n/base==0)
        base /= 10;

    while (n) {
        i = n/base;
        syscall_putch(i + '0');
        n -= base * i;
        base /= 10;
    }

}
int recur(int i)
{
    if (i)
        return recur(i-1);
    else
        return 0;
}

void test_kbd()
{
    u32int ret;
    s32int fd;
    char buf[1000];

    fd = syscall_open("/kbd", 0);

    while(1) {
        syscall_read(fd, buf, 4);
        ret = *(u32int*)buf;
        // key release
        if (!(ret & 0x10000000))
            syscall_putch(ret &0xff);
    }
}

void test_hda()
{
    u32int ret;
    s32int fd;
    char buf[1000];

    fd = syscall_open("/hda", 0);

    ret = syscall_read(fd, buf, 512);

}

void test_ramfs()
{
    u32int ret;
    s32int fd;
    char buf[1000] = {"miao "};
    char buf2[1000];

    fd = syscall_open("/a.txt", 0);
    ret = syscall_read(fd, buf2, 1000);
    syscall_close(fd);
    puts(buf2);

    fd = syscall_open("./././../a.txt", 0);
    ret = syscall_write(fd, buf, 5);
    syscall_close(fd);

    fd = syscall_open("/boot/../a.txt", 0);
    ret = syscall_read(fd, buf2, 1000);
    syscall_close(fd);
    puts(buf2);

    ret = syscall_chdir("modules/");
    fd = syscall_open(".", 0);
    ret = syscall_getdents(fd,buf2,1000);
    u32int i;
    for (i=0;i<ret;i+=36) {
        puts(buf2+i);
        puts("\n");
    }
    syscall_close(fd);

    ret = syscall_chdir("..");
    ret = syscall_getcwd(buf2, 100);
    puts("working dir: ");
    puts(buf2);
    puts("\n");

}

int main()
{
    char *s = "Init process running.\n";
    puts(s);

    test_ramfs();

    /*
    while (1) {
        puts(s);
        syscall_msleep(1000);
    }
    */

    /*
    fd=syscall_open("/a.txt",0);
    char *pa = (char*)syscall_mmap(0x30000000, 0x1000, 
            VMA_READ | VMA_WRITE | VMA_FILE,
            fd, 0x1000);
    u32int i;
    for (i=0; i<10; i++)
        pa[i] = s[i];
    pa[10] = 0;

    puts(pa);
    syscall_unmap(pa, 1000);
    puts(pa);
    while(1);

    ret = syscall_read(fd, buf, 1000);
    putn(ret);
    syscall_putch('\n');
    buf[ret] = 0;
    puts(buf);
    syscall_close(fd);
    */

    while(1);
}

