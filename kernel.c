#include "gdt.h"
#include "interrupt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

void mios_init(uint32_t magic, uint32_t ebx) {
  (void)magic;
  (void)ebx;

  init_gdt();
  init_idt();
  init_pic();

  init_terminal("mios\n");

  terminal_print("Hello breakpoints world.\n");

  while(1) {
    breakpoint();
    terminal_print("Forever...\n");
  }
}
