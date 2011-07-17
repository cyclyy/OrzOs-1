#ifndef SYSDEF_H
#define SYSDEF_H

#define always_inline __attribute__((always_inline)) inline

#ifndef NULL
#define NULL 0
#endif

#define PAGE_SIZE 4096
#define PAGE_LOG2_SIZE 12
#define PAGE_MASK 0xfffffffffffff000
#define PTR_SIZE 8

#define MMAP_DEV_START_ADDR 0xc0000000
#define MMAP_DEV_SIZE       0x40000000

#define KERNEL_STACK_TOP    0xffffffffe0000000
#define KERNEL_STACK_BOTTOM 0xffffffffd0000000
#define KERNEL_STACK_SIZE   0x2000
#define HIGHMEM_START_ADDR  0x100000
#define KERNEL_HEAP_START_ADDR 0xffffff0000000000      /* start from last 1024GB */
#define CODE_LOAD_ADDR  0xffffffffC0000000        /* kernel loaded here */
#define MAX_CODE_SIZE   0x1000000

#define USER_START_ADDR     0x0000020000000000
#define USER_CODE_START_ADDR USER_START_ADDR    
#define USER_CODE_END_ADDR  0x0000080000000000
#define USER_STACK_TOP      0x00000F0000000000
#define USER_STACK_BOTTOM   0x00000E0000000000
#define USER_STACK_SIZE     0x2000
#define USER_HEAP_START_ADDR USER_CODE_END_ADDR
#define USER_HEAP_END_ADDR  (USER_STACK_BOTTOM - 0x1000)
#define USER_END_ADDR       USER_STACK_TOP

#define MAX_NAME_LEN        100
#define MAX_IPC_PORT        1024
#define MAX_HANDLE_NUMBER   1024

typedef unsigned char   u8int;
typedef signed char     s8int;
typedef unsigned short  u16int;
typedef signed short    s16int;
typedef unsigned int    u32int;
typedef signed int      s32int;
typedef unsigned long   u64int;
typedef signed long     s64int;

#endif /* SYSDEF_H */
