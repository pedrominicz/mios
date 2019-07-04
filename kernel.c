#include "gdt.h"
#include "interrupt.h"
#include "syscall.h"
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

  syscall_print("This message will disappear.\n");
  breakpoint();

  // A bunch of slightly less ugly tests.
  syscall_clear();
  syscall_print("Hello slightly better syscalls world.\n");
  syscall_print_decimal(7654321);
  syscall_putchar('\n');
  syscall_print_hex(0x123);
  syscall_putchar('\n');
  syscall_print_hex8(0xef);
  syscall_putchar('\n');
  syscall_print_hex16(0xdead);
  syscall_putchar('\n');
  syscall_print_hex32(0x1cef70f1);
  syscall_putchar('\n');
  uint64_t all_digits = 0x01234567;
  all_digits <<= 32;
  all_digits |= 0x89abcdef;
  syscall_print_hex64(all_digits);
  syscall_putchar('\n');

  syscall_print("I will suffer a general protection fault... :(\n");
  breakpoint();

  while(1) {
    asm volatile ("hlt");
  }
}
