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

  terminal_print("Hello fancy interrupt error messages world.\n");

  switch_user_mode();

  *(char**)0 = "Paging fault.\n";

  while(1) {
    asm volatile ("hlt");
  }
}
