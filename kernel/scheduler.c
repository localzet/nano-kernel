#include "pit.h"
#include "scheduler.h"

#define MAX_TASKS 8
#define IDLE_PID 0

extern void context_restore(interrupt_frame_t* frame);

static task_t g_tasks[MAX_TASKS];
static int32_t g_task_count = 0;
static int32_t g_current = -1;

static uint8_t g_idle_stack[4096];

static void idle_task(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static interrupt_frame_t* task_build_initial_frame(uint8_t* stack_base, uint32_t stack_size, task_entry_t entry) {
    uint32_t* sp = (uint32_t*)(stack_base + stack_size);

    *--sp = 0x00000202;
    *--sp = 0x00000008;
    *--sp = (uint32_t)entry;

    *--sp = 0;
    *--sp = 0;

    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    *--sp = 0x10;
    *--sp = 0x10;
    *--sp = 0x10;
    *--sp = 0x10;

    return (interrupt_frame_t*)sp;
}

static int32_t scheduler_pick_next(void) {
    int32_t i;
    int32_t fallback_idle = -1;

    for (i = 1; i <= g_task_count; i++) {
        int32_t idx = (g_current + i) % g_task_count;
        task_t* t = &g_tasks[idx];

        if (t->state == TASK_SLEEPING && pit_get_ticks() >= t->sleep_until) {
            t->state = TASK_READY;
        }

        if (t->state == TASK_READY) {
            if (t->pid == IDLE_PID) {
                fallback_idle = idx;
            } else {
                return idx;
            }
        }
    }

    return fallback_idle;
}

void scheduler_init(void) {
    int32_t i;
    g_task_count = 0;
    g_current = -1;

    for (i = 0; i < MAX_TASKS; i++) {
        g_tasks[i].pid = -1;
        g_tasks[i].state = TASK_UNUSED;
        g_tasks[i].sleep_until = 0;
        g_tasks[i].frame = (interrupt_frame_t*)0;
        g_tasks[i].stack_base = (uint8_t*)0;
    }

    scheduler_create_task(IDLE_PID, idle_task, g_idle_stack, sizeof(g_idle_stack));
}

int32_t scheduler_create_task(int32_t pid, task_entry_t entry, uint8_t* stack_base, uint32_t stack_size) {
    task_t* t;
    if (g_task_count >= MAX_TASKS || stack_size < 512) {
        return -1;
    }

    t = &g_tasks[g_task_count++];
    t->pid = pid;
    t->state = TASK_READY;
    t->sleep_until = 0;
    t->stack_base = stack_base;
    t->frame = task_build_initial_frame(stack_base, stack_size, entry);
    return 0;
}

interrupt_frame_t* scheduler_on_timer(interrupt_frame_t* current) {
    int32_t next;

    if (g_current >= 0) {
        if (g_tasks[g_current].state == TASK_RUNNING) {
            g_tasks[g_current].state = TASK_READY;
        }
        g_tasks[g_current].frame = current;
    }

    next = scheduler_pick_next();
    if (next < 0) {
        return current;
    }

    g_current = next;
    g_tasks[g_current].state = TASK_RUNNING;
    return g_tasks[g_current].frame;
}

interrupt_frame_t* scheduler_on_sleep(interrupt_frame_t* current, uint32_t ticks) {
    int32_t next;

    if (g_current < 0) {
        current->eax = (uint32_t)-1;
        return current;
    }

    current->eax = 0;
    g_tasks[g_current].frame = current;
    g_tasks[g_current].sleep_until = pit_get_ticks() + ticks;
    g_tasks[g_current].state = TASK_SLEEPING;

    next = scheduler_pick_next();
    if (next < 0) {
        current->eax = (uint32_t)-1;
        return current;
    }

    g_current = next;
    g_tasks[g_current].state = TASK_RUNNING;
    return g_tasks[g_current].frame;
}

int32_t scheduler_current_pid(void) {
    if (g_current < 0) {
        return -1;
    }
    return g_tasks[g_current].pid;
}

void scheduler_start(void) {
    int32_t next = scheduler_pick_next();
    if (next < 0) {
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    g_current = next;
    g_tasks[g_current].state = TASK_RUNNING;
    context_restore(g_tasks[g_current].frame);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
