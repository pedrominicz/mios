#include "gdt.h"
#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

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

  switch_user_mode();

  asm volatile (
      "mov $0x1234, %%eax;"
      "int $0x80;"
      "mov $0x4321, %%eax;"
      "int $0x80" ::: "eax");

  while(1) { }
}
