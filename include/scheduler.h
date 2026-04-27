#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "interrupts.h"
#include "types.h"

typedef void (*task_entry_t)(void);

void scheduler_init(void);
void scheduler_add_task(int pid, task_entry_t entry);
void scheduler_start(void);

cpu_state_t* scheduler_on_tick(cpu_state_t* state);
cpu_state_t* scheduler_on_sleep(cpu_state_t* state, uint32_t ticks);
int scheduler_current_pid(void);

#endif
