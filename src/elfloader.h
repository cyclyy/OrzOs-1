#ifndef ELFLOADER_H
#define ELFLOADER_H

#include "sysdef.h"
#include "program.h"
#include "vmm.h"

s64int loadElfProgram(const char *path, struct Program *prog, struct VM *vm);

#endif /* ELFLOADER_H */
