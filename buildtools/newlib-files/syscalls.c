#include "sysdef.h"
#include "syscall.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

//#include <_ansi.h>
#include <errno.h>

// --- Process Control ---

int
_exit(int val){
    OzExitTask(val);
    return (-1);
}

int
execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}

/*
 * getpid -- only one process, so just return 1.
 */
#define __MYPID 1
    int
getpid()
{
    return __MYPID;
}


int 
fork(void) {
    errno = ENOTSUP;
    return -1;
}


/*
 * kill -- go out via exit...
 */
    int
kill(pid, sig)
    int pid;
    int sig;
{
    if(pid == __MYPID)
        _exit(sig);


    errno = EINVAL;
    return -1;
}

int
wait(int *status) {
    errno = ECHILD;
    return -1;
}

// --- I/O ---

/*
 * isatty -- returns 1 if connected to a terminal device,
 *           returns 0 if not. Since we're hooked up to a
 *           serial port, we'll say yes and return a 1.
 */
    int
isatty(fd)
    int fd;
{
    return (1);
}


int
close(int file) {
    return OzClose(file);
}

int
link(char *old, char *new) {
    errno = EMLINK;
    return -1;
}

int
lseek(int file, int ptr, int dir) {
    return OzSeek(file,ptr,dir);
}

int
open(const char *name, int flags, ...) {
    return OzOpen((char*)name,flags);
}

int
read(int file, char *ptr, int len) {
    // XXX: keyboard support
    return OzRead(file, len, ptr);
}

int 
fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int
stat(const char *file, struct stat *st){
    st->st_mode = S_IFCHR;
    return 0;
}

int
unlink(char *name) {
    errno = ENOENT;
    return -1;
}


int
write(int file, char *ptr, int len) {
    return OzWrite(file,len,ptr);
}

// --- Memory ---

/* _end is set in the linker command file */
//extern caddr_t _end;

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
caddr_t
sbrk(int nbytes){
    return (caddr_t)OzAddHeapSize(nbytes);
}


// --- Other ---
int gettimeofday(struct timeval *p, void *z){
    return -1;
}

