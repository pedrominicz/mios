OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
  . = 0x00400000;

  .text : { *(.multiboot) *(.text) }
  .data ALIGN(4096) : { *(.data .rodata*) }
  .bss ALIGN(4096) : { *(.bss) }
}
