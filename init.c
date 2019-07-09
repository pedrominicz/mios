#include "interrupt.h"
#include "memory.h"
#include "terminal.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

// Global descriptor table (GDT).
static uint64_t gdt[6] = {
  0x0000000000000000, // Null segment.
  0x00cf9a000000ffff, // Kernel code segment.
  0x00cf92000000ffff, // Kernel data segment.
  0x00cffa000000ffff, // User mode code segment.
  0x00cff2000000ffff, // User mode data segment.
  0x0000e90000000068, // Task state segment (TSS).
};

// Task state segment (TSS).
static uint32_t tss[26] = {
  [2] = KERNEL_DATA_SELECTOR, // Kernel stack segment.
  // Make segment registers accessible from privilege level-3, i.e. user mode.
  [18] = KERNEL_DATA_SELECTOR + 3, // Extra segment.
  [19] = KERNEL_CODE_SELECTOR + 3, // Code segment.
  [20] = KERNEL_DATA_SELECTOR + 3, // Stack segment.
  [21] = KERNEL_DATA_SELECTOR + 3, // Data segment.
  [22] = KERNEL_DATA_SELECTOR + 3, // Extra segment #2 (`fs`).
  [23] = KERNEL_DATA_SELECTOR + 3, // Extra segment #3 (`gs`).
  // Setting I/O map base address beyond the TSS limit and setting I/O
  // privilege level to 0 in `eflags` disables I/O instructions in user mode.
  [25] = 0xffff0000, // I/O map base address.
};

void init(void) {
  init_terminal();
  init_idt();

  init_kernel_page_directory();
  init_kernel_malloc();

  // Task state segment base address (bits 0-23).
  gdt[5] |= ((uint64_t)(uintptr_t)tss & 0x00ffffff) << 16;
  // Task state segment base address (bits 24-31).
  gdt[5] |= ((uint64_t)(uintptr_t)tss & 0xff000000) << 32;

  volatile uint64_t gdt_descriptor = 0;
  gdt_descriptor |= sizeof(gdt) - 1;
  gdt_descriptor |= (uint64_t)(uintptr_t)gdt << 16;

  asm volatile ("lgdt (%0)" :: "r"(&gdt_descriptor));

  asm volatile ("int $0x30");

  printf("Hello.\n");
  printf("Unsigned decimal:     %u\n"
         "Unsigned hexadecimal: %x\n", 1234, 0xabcd);
  printf("Long unsigned decimal:     %zu\n"
         "Long unsigned hexadecimal: %zx\n"
         "Pointer: %p\n", 123456789000, 0x1cbe991a08, init);
  printf("String: %s\n", "Hello!");
  printf("Null string: %s\n", NULL);
  printf("Unsigned decimal with padding:     %10u\n"
         "Unsigned hexadecimal with padding: %10x\n", 1234, 0xabcd);
  printf("Unsigned decimal with zero padding:     %010u\n"
         "Unsigned hexadecimal with zero padding: %010x\n", 1234, 0xabcd);
  printf("Unsigned decimal with space padding:     % 10u\n"
         "Unsigned hexadecimal with space padding: % 10x\n", 1234, 0xabcd);

  asm volatile ("int $0x9");

  while(1) {
    asm volatile ("hlt");
  }
}
