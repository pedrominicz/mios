#ifndef MIOS_MEMORY_H
#define MIOS_MEMORY_H

#define KERNEL_PHYSICAL_BASE    0x00100000 // 1MB.
#define KERNEL_VIRTUAL_BASE     0xf0100000
#define KERNEL_OFFSET           (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)

#define MAX_MEMORY_REGIONS 16

#ifndef __ASSEMBLER__

void init_memory(void);

#endif // __ASSEMBLER__

#endif // MIOS_MEMORY_H
