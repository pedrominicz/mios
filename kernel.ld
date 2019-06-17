OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(start)

SECTIONS
{
  . = 0x80000;

  .text : AT(0x80000) { *(.start .text) } =0x90
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }

  .bss : { *(.bss) }
}
