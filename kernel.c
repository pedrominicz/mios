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
  init_interrupt();

  terminal_clear();
  asm volatile ("int 0x03");
  terminal_print("Hello interrupt world!\n");

  hang();
}
