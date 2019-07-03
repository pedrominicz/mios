#include "gdt.h"
#include "interrupt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

extern void switch_user_mode(void); // Defined in "gdt.S".

void interrupt_handler(InterruptFrame* frame) {
  terminal_print("Interrupt 0x");
  terminal_print_hex(frame->interrupt_number);
  terminal_print(". Error code 0x");
  terminal_print_hex(frame->interrupt_error_code);
  terminal_print(".\n");
  asm volatile ("hlt");
}

void syscall_handler(InterruptFrame* frame) {
  terminal_print("Syscall 0x");
  terminal_print_hex(frame->eax);
  terminal_print(".\n");
}

void mios_init(uint32_t magic, uint32_t ebx) {
  (void)magic;
  (void)ebx;

  init_gdt();
  init_idt();
  init_pic();

  init_terminal("mios\n");

  // Horribly ugly tests.
  terminal_print("1234567890123456789012345678901234567890\n");
  terminal_print("Hello\ttabs\tworld.\n");
  for(size_t i = 0; i < 100; ++i) {
    terminal_print_hex(i);
    terminal_putchar('\n');
  }
  terminal_print("Hello\ttabs\tworld.\n");
  terminal_putchar('\n');
  terminal_putchar('\n');
  for(size_t i = 0; i < 10; ++i) {
    terminal_print_hex(i);
    terminal_putchar('\n');
  }

  asm volatile ("hlt");
}
