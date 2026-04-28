#include "fs.h"

#define INITRD_MAGIC 0x4E4B5244u
#define INITRD_MAX_FILES 32u
#define INITRD_NAME_LEN 32u

typedef struct initrd_header {
    uint32_t magic;
    uint32_t file_count;
} __attribute__((packed)) initrd_header_t;

typedef struct initrd_entry {
    char name[INITRD_NAME_LEN];
    uint32_t offset;
    uint32_t size;
} __attribute__((packed)) initrd_entry_t;

static uint32_t g_fs_start = 0;
static uint32_t g_fs_end = 0;
static uint32_t g_fs_file_count = 0;
static fs_file_t g_files[INITRD_MAX_FILES];

static int32_t fs_strcmp(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) {
            return (int32_t)((uint8_t)*a - (uint8_t)*b);
        }
        a++;
        b++;
    }

    return (int32_t)((uint8_t)*a - (uint8_t)*b);
}

void fs_init(uint32_t start, uint32_t end) {
    const initrd_header_t* header;
    const initrd_entry_t* entries;
    uint32_t i;

    g_fs_start = start;
    g_fs_end = end;
    g_fs_file_count = 0;

    if (start == 0 || end <= start) {
        return;
    }

    if ((end - start) < sizeof(initrd_header_t)) {
        return;
    }

    header = (const initrd_header_t*)(uintptr_t)start;
    if (header->magic != INITRD_MAGIC) {
        return;
    }

    if (header->file_count > INITRD_MAX_FILES) {
        return;
    }

    if ((end - start) < sizeof(initrd_header_t) + (header->file_count * sizeof(initrd_entry_t))) {
        return;
    }

    entries = (const initrd_entry_t*)(uintptr_t)(start + sizeof(initrd_header_t));
    for (i = 0; i < header->file_count; i++) {
        uint32_t data_start = start + entries[i].offset;
        uint32_t data_end = data_start + entries[i].size;
        uint32_t j;

        if (entries[i].offset < sizeof(initrd_header_t) + (header->file_count * sizeof(initrd_entry_t))) {
            return;
        }

        if (data_end < data_start || data_end > end) {
            return;
        }

        for (j = 0; j < INITRD_NAME_LEN; j++) {
            if (entries[i].name[j] == '\0') {
                break;
            }
        }

        if (j == INITRD_NAME_LEN) {
            return;
        }

        g_files[i].name = entries[i].name;
        g_files[i].data = (const uint8_t*)(uintptr_t)data_start;
        g_files[i].size = entries[i].size;
    }

    g_fs_file_count = header->file_count;
}

void fs_list(fs_list_cb_t cb, void* user) {
    uint32_t i;
    if (cb == (fs_list_cb_t)0) {
        return;
    }

    for (i = 0; i < g_fs_file_count; i++) {
        cb(&g_files[i], user);
    }
}

const fs_file_t* fs_find(const char* name) {
    uint32_t i;

    if (name == (const char*)0) {
        return (const fs_file_t*)0;
    }

    for (i = 0; i < g_fs_file_count; i++) {
        if (fs_strcmp(g_files[i].name, name) == 0) {
            return &g_files[i];
        }
    }

    return (const fs_file_t*)0;
}

int32_t fs_read(const char* name, const uint8_t** data, uint32_t* size) {
    const fs_file_t* file = fs_find(name);

    if (file == (const fs_file_t*)0 || data == (const uint8_t**)0 || size == (uint32_t*)0) {
        return -1;
    }

    *data = file->data;
    *size = file->size;
    return 0;
}
