#ifndef SCHEDULE_H
#define SCHEDULE_H

struct Task;

void schedule();

void rqAdd(struct Task *task);

void rqRemove(struct Task *task);

#endif /* SCHEDULE_H */
