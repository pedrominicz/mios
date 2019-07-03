#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stdbool.h>
#include <stdint.h>

static volatile bool breakpoint_continue = false;

void interrupt_handler(InterruptFrame* frame) {
  terminal_print("Interrupt 0x");
  terminal_print_hex(frame->interrupt_number);
  terminal_print(". Error code 0x");
  terminal_print_hex(frame->interrupt_error_code);
  terminal_print(".\n");
}

void breakpoint_handler(InterruptFrame* frame) {
  terminal_print("Breakpoint at \"");
  terminal_print((const char*)frame->eax);
  terminal_print("\" line #");
  terminal_print_decimal(frame->ebx);
  terminal_print(".\n");

  asm volatile ("sti");
  breakpoint_continue = false;
  while(!breakpoint_continue) { }
}

void keyboard_handler(InterruptFrame* frame) {
  (void)frame;

  terminal_print("Keyboard interrupt.\n");

  if(inb(0x60) == 0x9c) { // Enter key released.
    breakpoint_continue = true;
  }
}

void syscall_handler(InterruptFrame* frame) {
  terminal_print("Syscall 0x");
  terminal_print_hex(frame->eax);
  terminal_print(".\n");
}
