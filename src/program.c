#include "program.h"
#include "util.h"
#include "kmm.h"

struct Program *progCreate()
{
    struct Program *prog;
    prog = (struct Program *)kMalloc(sizeof(struct Program));
    memset(prog,0,sizeof(struct Program));
    return prog;
}

struct Program *progRef(struct Program *prog)
{
    prog->ref++;
    return prog;
}

s64int progDeref(struct Program *prog)
{
    if (prog->ref <= 0)
        return -1;
    --prog->ref;
    if (prog->ref==0)
        kFree(prog);
    return 0;
}


