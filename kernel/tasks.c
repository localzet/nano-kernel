#include "scheduler.h"
#include "shell.h"
#include "syscall.h"
#include "tasks.h"

static uint8_t g_task_a_stack[4096];
static uint8_t g_task_b_stack[4096];
static uint8_t g_shell_stack[4096];

static void write_pid_prefix(const char* prefix, int32_t pid) {
    char buf[16];
    int i = 0;
    uint32_t v = (uint32_t)pid;

    if (pid < 0) {
        sys_write("pid=-1");
        return;
    }

    sys_write(prefix);

    if (v == 0) {
        sys_write("0");
        return;
    }

    while (v > 0) {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }

    while (i > 0) {
        char ch[2];
        ch[0] = buf[--i];
        ch[1] = '\0';
        sys_write(ch);
    }
}

static void task_a(void) {
    for (;;) {
        int32_t pid = sys_getpid();
        sys_write("[task 1] pid=");
        write_pid_prefix("", pid);
        sys_write(" hello from task A\n");
        sys_sleep(40);
    }
}

static void u32_write(uint32_t value) {
    char buf[16];
    int i = 0;

    if (value == 0) {
        sys_write("0");
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        char ch[2];
        ch[0] = buf[--i];
        ch[1] = '\0';
        sys_write(ch);
    }
}

static void task_b(void) {
    uint32_t counter = 0;
    for (;;) {
        counter++;
        sys_write("[task 2] pid=");
        u32_write((uint32_t)sys_getpid());
        sys_write(" counter=");
        u32_write(counter);
        sys_write("\n");
        sys_sleep(60);
    }
}

void tasks_init(void) {
    scheduler_create_task(1, task_a, g_task_a_stack, sizeof(g_task_a_stack));
    scheduler_create_task(2, task_b, g_task_b_stack, sizeof(g_task_b_stack));
    scheduler_create_task(3, shell_task_entry, g_shell_stack, sizeof(g_shell_stack));
}
