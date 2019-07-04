#include "interrupt.h"
#include "syscall.h"
#include "terminal.h"

#include <stdint.h>

void syscall_handler(const InterruptFrame* const frame) {
  switch(frame->eax) {
  case SYSCALL_CLEAR:
    terminal_clear();
    break;
  case SYSCALL_PRINT_DECIMAL: {
    const uint64_t n = (uint64_t)frame->ecx << 32 | frame->ebx;
    terminal_print_decimal(n);
  } break;
  case SYSCALL_PRINT_HEX: {
    const uint64_t n = (uint64_t)frame->ecx << 32 | frame->ebx;
    terminal_print_hex(n);
  } break;
  case SYSCALL_PRINT_HEX8:
    terminal_print_hex8(frame->ebx);
    break;
  case SYSCALL_PRINT_HEX16:
    terminal_print_hex16(frame->ebx);
    break;
  case SYSCALL_PRINT_HEX32:
    terminal_print_hex32(frame->ebx);
    break;
  case SYSCALL_PRINT_HEX64: {
    const uint64_t n = (uint64_t)frame->ecx << 32 | frame->ebx;
    terminal_print_hex64(n);
  } break;
  case SYSCALL_PUTCHAR:
    terminal_putchar(frame->ebx);
    break;
  case SYSCALL_PRINT:
    terminal_print((const char*)frame->ebx);
    break;
  }
}
