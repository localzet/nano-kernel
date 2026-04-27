#include "console.h"
#include "scheduler.h"

#define MAX_TASKS 3
#define TASK_STACK_SIZE 4096

typedef struct task {
    int pid;
    uint32_t sleep_ticks;
    cpu_state_t* context;
    uint8_t stack[TASK_STACK_SIZE];
} task_t;

extern void start_first_task(uint32_t task_esp);

static task_t tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = -1;

static cpu_state_t* build_initial_context(task_t* task, task_entry_t entry) {
    uint32_t* sp = (uint32_t*)(task->stack + TASK_STACK_SIZE);

    *--sp = 0x00000202;          // eflags
    *--sp = 0x00000008;          // cs
    *--sp = (uint32_t)entry;     // eip

    *--sp = 0;                   // eax
    *--sp = 0;                   // ecx
    *--sp = 0;                   // edx
    *--sp = 0;                   // ebx
    *--sp = 0;                   // esp_dummy
    *--sp = 0;                   // ebp
    *--sp = 0;                   // esi
    *--sp = 0;                   // edi

    *--sp = 0x10;                // ds
    *--sp = 0x10;                // es
    *--sp = 0x10;                // fs
    *--sp = 0x10;                // gs

    return (cpu_state_t*)sp;
}

void scheduler_init(void) {
    task_count = 0;
    current_task = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].pid = 0;
        tasks[i].sleep_ticks = 0;
        tasks[i].context = 0;
    }
}

void scheduler_add_task(int pid, task_entry_t entry) {
    if (task_count >= MAX_TASKS) {
        return;
    }

    tasks[task_count].pid = pid;
    tasks[task_count].sleep_ticks = 0;
    tasks[task_count].context = build_initial_context(&tasks[task_count], entry);
    task_count++;
}

static void scheduler_tick_sleepers(void) {
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].sleep_ticks > 0) {
            tasks[i].sleep_ticks--;
        }
    }
}

static int scheduler_find_next_runnable(int start) {
    if (task_count == 0) {
        return -1;
    }

    for (int i = 1; i <= task_count; i++) {
        int idx = (start + i) % task_count;
        if (tasks[idx].sleep_ticks == 0) {
            return idx;
        }
    }
    return start;
}

cpu_state_t* scheduler_on_tick(cpu_state_t* state) {
    if (task_count == 0) {
        return state;
    }

    if (current_task >= 0) {
        tasks[current_task].context = state;
    }

    scheduler_tick_sleepers();

    int next = scheduler_find_next_runnable(current_task < 0 ? 0 : current_task);
    if (next < 0) {
        return state;
    }

    current_task = next;
    return tasks[current_task].context;
}

cpu_state_t* scheduler_on_sleep(cpu_state_t* state, uint32_t ticks) {
    if (current_task < 0 || current_task >= task_count) {
        state->eax = (uint32_t)-1;
        return state;
    }

    tasks[current_task].context = state;
    tasks[current_task].sleep_ticks = ticks;
    state->eax = 0;

    int next = scheduler_find_next_runnable(current_task);
    if (next == current_task) {
        return state;
    }

    current_task = next;
    return tasks[current_task].context;
}

int scheduler_current_pid(void) {
    if (current_task < 0 || current_task >= task_count) {
        return -1;
    }
    return tasks[current_task].pid;
}

void scheduler_start(void) {
    if (task_count == 0) {
        console_write("[Scheduler] No tasks\n");
        return;
    }

    console_write("[Scheduler] Started\n");
    current_task = 0;
    start_first_task((uint32_t)tasks[current_task].context);
}
