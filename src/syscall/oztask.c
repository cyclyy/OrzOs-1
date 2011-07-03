#include "oztask.h"
#include "sysdef.h"
#include "kmm.h"
#include "vmm.h"
#include "task.h"
#include "program.h"
#include "util.h"

s64int OzNewTask(char *path, u64int flags)
{
    char s[MAX_NAME_LEN];
    copyFromUser(s,path,MAX_NAME_LEN);
    s[MAX_NAME_LEN-1] = 0;
    return kNewTask(s,flags);
}

s64int OzExitTask(s64int status)
{
    kExitTask(status);
    return 0;
}

u64int OzAddHeapSize(s64int incr)
{
    struct Program *prog;
    u64int oldbrk, newAllocBefore;
    s64int ret;

    prog = currentTask->prog;
    if (!prog || (prog->brk + incr < USER_HEAP_START_ADDR) || 
            (prog->brk + incr >= USER_HEAP_END_ADDR)) {
        return 0;
    }
    if (incr == 0) {
        return prog->brk;
    } else if (incr < 0) {
        oldbrk = prog->brk;
        prog->brk += incr;
        return oldbrk;
    } else {
        oldbrk = prog->brk;
        prog->brk += incr;
        ret = 0;
        if (prog->brk > prog->allocBefore) {
            newAllocBefore = (prog->brk + PAGE_SIZE - 1) & PAGE_MASK;
            ret = vmAddArea(currentTask->vm, prog->allocBefore, 
                    newAllocBefore - prog->allocBefore,
                    VMA_STATUS_USED | VMA_OWNER_USER | VMA_TYPE_HEAP);
            prog->allocBefore = newAllocBefore;
        }
        if (ret == 0)
            return oldbrk;
        else {
            return 0;
        }
    }

}

// vim: sw=4 sts=4 et tw=100
