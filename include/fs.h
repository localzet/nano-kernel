#ifndef NANOKERNEL_FS_H
#define NANOKERNEL_FS_H

#include "types.h"

typedef struct fs_file {
    const char* name;
    const uint8_t* data;
    uint32_t size;
} fs_file_t;

typedef void (*fs_list_cb_t)(const fs_file_t* file, void* user);

void fs_init(uint32_t start, uint32_t end);
void fs_list(fs_list_cb_t cb, void* user);
const fs_file_t* fs_find(const char* name);
int32_t fs_read(const char* name, const uint8_t** data, uint32_t* size);

#endif
