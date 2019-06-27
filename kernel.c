#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

void mios_init(void) {
  init_terminal();
  terminal_print("Hello GRUB world!\n");

  while(1) {
    asm volatile ("cli");
    asm volatile ("hlt");
  }
}
