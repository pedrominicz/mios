#include "interrupt.h"
#include "memory.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

void init(void) {
  init_kernel_page_directory();
  init_kernel_malloc();
  init_gdt(); // Global descriptor table (GDT).
  init_idt(); // Interrupt descriptor table (IDT).

  asm volatile ("int $3");

  while(1) {
    asm volatile ("hlt");
  }
}
