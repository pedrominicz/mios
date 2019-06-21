#include "interrupt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

static inline void hang(void) {
  while(1) {
    asm volatile ("cli");
    asm volatile ("hlt");
  }
}

void mios_init(void) {
  init_idt();

  terminal_clear();
  asm volatile (
      "mov eax, 0x73706172;"
      "mov ebx, 0x74206576;"
      "mov ecx, 0x6f6c2069;"
      "int 0x03" ::: "eax", "ebx", "ecx");
  terminal_print("Hello interrupt world!\n");

  hang();
}
