#include "memory.h"

OUTPUT_ARCH("i386")
ENTRY(entry)

SECTIONS
{
  . = KERNEL_VIRTUAL_BASE;

  .text : AT(ADDR(.text) - KERNEL_OFFSET) { *(.multiboot) *(.text) }
  .data : AT(ADDR(.data) - KERNEL_OFFSET) { *(.rodata* .data) }
  .bss : AT(ADDR(.bss) - KERNEL_OFFSET) { *(.bss) }

  ASSERT(. < KERNEL_VIRTUAL_BASE + 0x300000, "Kernel bigger than 3MB.")
}
