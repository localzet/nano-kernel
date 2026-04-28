#include "paging.h"

#define PAGE_SIZE 4096u
#define PAGE_PRESENT 0x001u
#define PAGE_RW 0x002u

#define IDENTITY_TABLE_COUNT 4u
#define IDENTITY_MAP_SIZE (IDENTITY_TABLE_COUNT * 1024u * PAGE_SIZE)

extern uint8_t kernel_start;
extern uint8_t kernel_end;

static uint32_t g_page_directory[1024] __attribute__((aligned(4096)));
static uint32_t g_page_tables[IDENTITY_TABLE_COUNT][1024] __attribute__((aligned(4096)));

static void paging_load_cr3(uint32_t page_directory_phys) {
    __asm__ volatile ("mov %0, %%cr3" :: "r"(page_directory_phys) : "memory");
}

static void paging_enable_pg_bit(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");
}

void paging_init(void) {
    uint32_t i;
    uint32_t t;

    for (i = 0; i < 1024; i++) {
        g_page_directory[i] = 0;
    }

    for (t = 0; t < IDENTITY_TABLE_COUNT; t++) {
        for (i = 0; i < 1024; i++) {
            uint32_t phys = (t * 1024u + i) * PAGE_SIZE;
            g_page_tables[t][i] = phys | PAGE_PRESENT | PAGE_RW;
        }

        g_page_directory[t] = ((uint32_t)(uintptr_t)&g_page_tables[t][0]) | PAGE_PRESENT | PAGE_RW;
    }

    paging_load_cr3((uint32_t)(uintptr_t)&g_page_directory[0]);
    paging_enable_pg_bit();
}

bool paging_is_enabled(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000u) != 0;
}

uint32_t paging_identity_start(void) {
    return 0;
}

uint32_t paging_identity_end(void) {
    return IDENTITY_MAP_SIZE - 1u;
}

uint32_t paging_kernel_start(void) {
    return (uint32_t)(uintptr_t)&kernel_start;
}

uint32_t paging_kernel_end(void) {
    return (uint32_t)(uintptr_t)&kernel_end;
}
