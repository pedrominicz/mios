#ifndef MIOS_MEMORY_H
#define MIOS_MEMORY_H

#define KERNEL_PHYSICAL_BASE    0x00100000 // 1MB.
#define KERNEL_VIRTUAL_BASE     0xf0100000
#define KERNEL_OFFSET           (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)

#define KERNEL_CODE_SELECTOR    0x0008
#define KERNEL_DATA_SELECTOR    0x0010

#ifndef __ASSEMBLER__

#include <stdint.h>

void init_kernel_page_directory(void);
void init_kernel_malloc(void);

void switch_kernel_page_directory(void);

static inline uint32_t virtual_to_physical(void* address) {
  return (uint32_t)address - KERNEL_OFFSET;
}

static inline void* physical_to_virtual(uint32_t address) {
  return (void*)(address + KERNEL_OFFSET);
}

#endif // __ASSEMBLER__

#endif // MIOS_MEMORY_H
