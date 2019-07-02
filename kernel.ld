OUTPUT_ARCH(i386)
ENTRY(_start)

_kernel_physical_base = 0x00100000; /* 1MB. */
_kernel_virtual_base = 0xf0100000;
_kernel_offset = (_kernel_virtual_base - _kernel_physical_base);

SECTIONS
{
  /* The code in the `.multiboot` section sets up basic paging */
  . = 0xf0100000;

  .text ALIGN(4096) : AT(ADDR(.text) - _kernel_offset) {
    *(.multiboot) *(.text)
  }
  .data ALIGN(4096) : AT(ADDR(.data) - _kernel_offset) { *(.data .rodata*) }
  .bss ALIGN(4096) : AT(ADDR(.bss) - _kernel_offset) { *(.bss) }
}
