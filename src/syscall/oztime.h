#ifndef OZTIME_H
#define OZTIME_H

#include "sysdef.h"

unsigned long OzMilliAlarm(unsigned long msec);

unsigned long OzMilliSleep(unsigned long msec);

unsigned long OzGetTicks();

#endif /* OZTIME_H */
