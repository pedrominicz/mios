#include "gdt.h"
#include "idt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

extern uint64_t gdt[6]; // Defined in "gdt.S".

void trap(void) {
  terminal_print("Trap.\n");
}

void mios_init(uint32_t magic, uint32_t ebx) {
  (void)magic;
  (void)ebx;

  init_gdt();
  init_idt();

  init_terminal();
  terminal_print("Hello weird C preprocessor macros world!\n");
  terminal_print_hex_pad(gdt[0]);
  terminal_putchar('\n');
  terminal_print_hex_pad(gdt[1]);
  terminal_putchar('\n');
  terminal_print_hex_pad(gdt[2]);
  terminal_putchar('\n');
  terminal_print_hex_pad(gdt[3]);
  terminal_putchar('\n');
  terminal_print_hex_pad(gdt[4]);
  terminal_putchar('\n');
  terminal_print_hex_pad(gdt[5]);
  terminal_putchar('\n');

  asm volatile ("int $3");

  while(1) {
    asm volatile ("hlt");
  }
}
