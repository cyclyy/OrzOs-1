#ifndef TIMER_H
#define TIMER_H

#include "common.h"

#define HZ            100

extern u32int ticks;

void init_timer(u32int freq);

#endif

