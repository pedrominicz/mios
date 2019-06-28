#include "gdt.h"
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

  init_terminal();
  terminal_print("Hello GRUB world!\n");

  while(1) {
    asm volatile ("hlt");
  }
}
