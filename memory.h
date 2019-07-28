#ifndef MIOS_MEMORY_H
#define MIOS_MEMORY_H

#define KERNEL_PHYSICAL_BASE    0x00100000 // 1MB.
#define KERNEL_VIRTUAL_BASE     0xf0100000
#define KERNEL_OFFSET           (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)

#define KERNEL_CODE_SEGMENT     0x0008
#define KERNEL_DATA_SEGMENT     0x0010
#define USER_CODE_SEGMENT       (0x0018 | 3)
#define USER_DATA_SEGMENT       (0x0020 | 3)
#define TASK_STATE_SEGMENT      (0x0028 | 3)

#ifndef __ASSEMBLER__

#include <stdint.h>

extern uint32_t kernel_page_directory[1024]; // Defined in "entry.S".

void init_kernel_page_directory(void);
void init_kernel_malloc(void);

void free_page(void* page);
void* malloc_page(void);

static inline uintptr_t virtual_to_physical(const void* const address) {
  return (uintptr_t)address - KERNEL_OFFSET;
}

static inline void* physical_to_virtual(const uintptr_t address) {
  return (void*)(address + KERNEL_OFFSET);
}

static inline void switch_page_directory(const uint32_t* const page_directory) {
  asm volatile ("mov %0, %%cr3" :: "r"(virtual_to_physical(page_directory)));
}

#endif // __ASSEMBLER__

#endif // MIOS_MEMORY_H
