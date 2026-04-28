#ifndef NANOKERNEL_SCHEDULER_H
#define NANOKERNEL_SCHEDULER_H

#include "interrupts.h"
#include "types.h"

typedef void (*task_entry_t)(void);

typedef enum task_state {
    TASK_UNUSED = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING
} task_state_t;

typedef struct task {
    int32_t pid;
    task_state_t state;
    uint32_t sleep_until;
    interrupt_frame_t* frame;
    uint8_t* stack_base;
} task_t;

typedef struct scheduler_task_info {
    int32_t pid;
    task_state_t state;
    uint32_t sleep_until;
} scheduler_task_info_t;

void scheduler_init(void);
int32_t scheduler_create_task(int32_t pid, task_entry_t entry, uint8_t* stack_base, uint32_t stack_size);
void scheduler_start(void);

interrupt_frame_t* scheduler_on_timer(interrupt_frame_t* current);
interrupt_frame_t* scheduler_on_sleep(interrupt_frame_t* current, uint32_t ticks);
int32_t scheduler_current_pid(void);
int32_t scheduler_task_count(void);
int32_t scheduler_get_task_info(int32_t index, scheduler_task_info_t* out);

#endif
