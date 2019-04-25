#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "config.h"

void schedule(void) __attribute__ ( ( naked ) );
void run(void) __attribute__ ( ( naked ) );
void task_create(void *f);

#endif
