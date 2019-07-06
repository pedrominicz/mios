#include "memory.h"

OUTPUT_ARCH("i386")
ENTRY(entry)

SECTIONS
{
  . = KERNEL_PHYSICAL_BASE;

  .unpaged.text : { unpaged*(.text) }
  .unpaged.data : { unpaged*(.data .rodata*) }
  .unpaged.bss : { unpaged*(.bss) }

  . += KERNEL_OFFSET;

  .text : AT(ADDR(.text) - KERNEL_OFFSET) { *(.multiboot) *(.text) }
  .data : AT(ADDR(.data) - KERNEL_OFFSET) { *(.data .rodata*) }
  .bss : AT(ADDR(.bss) - KERNEL_OFFSET) { *(.bss) }

  PROVIDE(kernel_end = .);
}
