#ifndef NANOKERNEL_PAGING_H
#define NANOKERNEL_PAGING_H

#include "types.h"

void paging_init(uint32_t min_map_end);
bool paging_is_enabled(void);
uint32_t paging_identity_start(void);
uint32_t paging_identity_end(void);
uint32_t paging_kernel_start(void);
uint32_t paging_kernel_end(void);

#endif
