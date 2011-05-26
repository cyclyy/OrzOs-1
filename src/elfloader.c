#include "elfloader.h"
#include "sysdef.h"
#include "elf.h"
#include "program.h"
#include "util.h"
#include "vfs.h"
#include "vmm.h"
#include "kmm.h"
#include "task.h"

s64int isValidElfHeader(Elf64_Ehdr *ehdr)
{
    s64int ret;

    if (!ehdr)
        return 0;

    ret = 1;

    ret &= (ehdr->e_ident[EI_MAG0] == ELFMAG0);
    ret &= (ehdr->e_ident[EI_MAG1] == ELFMAG1);
    ret &= (ehdr->e_ident[EI_MAG2] == ELFMAG2);
    ret &= (ehdr->e_ident[EI_MAG3] == ELFMAG3);
    ret &= (ehdr->e_ident[EI_CLASS] == ELFCLASS64);
    ret &= (ehdr->e_ident[EI_DATA] == ELFDATA2LSB);
    ret &= (ehdr->e_ident[EI_VERSION] == EV_CURRENT);

    ret &= (ehdr->e_type == ET_EXEC);
    ret &= (ehdr->e_machine == EM_X86_64);
    ret &= ((ehdr->e_entry >= USER_START_ADDR) && (ehdr->e_entry < USER_END_ADDR));
    ret &= (ehdr->e_phnum > 0);

    return ret;
}

s64int loadElfProgram(const char *path, struct Program *prog, struct VM *vm)
{
    struct VNode node;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    s64int ret, n;
    u64int len, i, j, size, remain;
    char *buffer;

    ret = vfsOpen(path, 0, &node);

    if (ret!=0)
        return ret;

    ehdr = (Elf64_Ehdr *)kMalloc(sizeof(Elf64_Ehdr));
    if (!ehdr)
        return -1;
    buffer = (char*)kMallocEx(PAGE_SIZE, 1, 0);
    n = vfsRead(&node, 0, sizeof(Elf64_Ehdr), ehdr);

    if ((n == sizeof(Elf64_Ehdr)) && isValidElfHeader(ehdr)) {
        prog->entry = ehdr->e_entry;
        len = ehdr->e_phentsize * ehdr->e_phnum;
        phdr = (Elf64_Phdr*)kMalloc(len);
        n = vfsRead(&node, ehdr->e_phoff, len, phdr);
        if (n==len) {
            for (i=0; i<ehdr->e_phnum; i++) {
                if (phdr[i].p_type == PT_LOAD) {
                    ret = vmAddArea(vm, phdr[i].p_vaddr, phdr[i].p_memsz,
                            VMA_TYPE_HEAP | VMA_OWNER_USER | VMA_STATUS_USED);
                    if (ret!=0) {
                        break;
                    }

                    size = phdr[i].p_filesz;
                    remain = size & 0xfff;
                    size -= remain;
                    for (j=0; j<size; j+=PAGE_SIZE) {
                        len = vfsRead(&node, phdr[i].p_offset+j, PAGE_SIZE, buffer);
                        if (len!=PAGE_SIZE) {
                            ret = -1;
                            break;
                        }
                        DBG("%x,%x",vm,currentTask->vm);
                        vmemcpy(vm, (void*)(phdr[i].p_vaddr+j), currentTask->vm, buffer, PAGE_SIZE);
                    }
                    if (remain) {
                        len = vfsRead(&node, phdr[i].p_offset+size, remain, buffer);
                        if (len == remain)
                            vmemcpy(vm, (void*)(phdr[i].p_vaddr+size), currentTask->vm, buffer, remain);
                        else 
                            ret = -1;
                    }
                    if (ret!=0)
                        break;
                }
            }
        }
        // setup stack
        if (ret == 0) {
            ret = vmAddArea(vm, USER_STACK_TOP-USER_STACK_SIZE,
                    USER_STACK_SIZE, VMA_TYPE_STACK | VMA_OWNER_USER | VMA_STATUS_USED);

        }

        kFree(phdr);
    } else 
        ret = -1;

    kFree(buffer);
    kFree(ehdr);

    vfsClose(&node);

    return ret;
}

