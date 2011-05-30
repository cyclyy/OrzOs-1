#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "task.h"

void schedule();

void rqAdd(struct Task *task);

void rqRemove(struct Task *task);

#endif /* SCHEDULE_H */
