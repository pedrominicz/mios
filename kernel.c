#include "gdt.h"
#include "idt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

void trap(void) {
  terminal_print("Trap.\n");
}

void mios_init(uint32_t magic, uint32_t ebx) {
  (void)magic;
  (void)ebx;

  init_gdt();
  init_idt();

  init_terminal();
  terminal_print("Hello trap world!\n");

  asm volatile ("int $3");

  while(1) {
    asm volatile ("hlt");
  }
}
