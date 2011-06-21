#include "oztask.h"
#include "sysdef.h"
#include "kmm.h"
#include "vmm.h"
#include "task.h"
#include "util.h"

s64int OzNewTask(char *path, u64int flags)
{
    char s[MAX_NAME_LEN];
    copyFromUser(s,path,MAX_NAME_LEN);
    s[MAX_NAME_LEN-1] = 0;
    return kNewTask(s,flags);
}

// vim: sw=4 sts=4 et tw=100
