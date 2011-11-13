#include "sysdef.h"

#define DYNAMIC_LOWMEM_START    0x1000
#define DYNAMIC_LOWMEM_END      0x7000
#define DYNAMIC_LOWMEM_SIZE     0x6000  // 24KB

void initRealModeInterface();

void realModeInterrupt(u8int num);

// manage 0x1000 - 0x6ffff, 24KB low memory
void *realModeAlloc(u64int size);

void realModeFree(void *ptr);
