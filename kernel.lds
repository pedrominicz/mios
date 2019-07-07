#include "memory.h"

OUTPUT_ARCH("i386")
ENTRY(entry)

SECTIONS
{
  . = KERNEL_VIRTUAL_BASE;

  .text : AT(ADDR(.text) - KERNEL_OFFSET) { *(.multiboot .text) }
  .data : AT(ADDR(.data) - KERNEL_OFFSET) { *(.rodata* .data) }
  .bss : AT(ADDR(.bss) - KERNEL_OFFSET) { *(.bss) }

  kernel_end = .;
}
