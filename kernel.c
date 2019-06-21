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
  terminal_clear();

  int a, b;
  asm volatile ("mov %0, 1" : "=r"(a));
  asm volatile ("mov %0, 0" : "=r"(b));
  if(a != 1 || b != 0) hang();

  terminal_print("Hello kernel world!\n");
  hang();
}
