#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stddef.h>
#include <stdint.h>

void mios_init(void) {
  init_interrupt();
  init_terminal();

  terminal_clear();
  terminal_print("Hello interrupt world!\n");
  terminal_print_hex(0x1234);
  terminal_putchar('\n');
  terminal_print_hex(0xdead);
  terminal_putchar('\n');
  terminal_print_hex(0xc0de);
  terminal_putchar('\n');

  // Start the programmable interval timer (PIT).
  const uint32_t frequency = 50;
  const uint32_t divisor = 1193180 / frequency;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xff);
  outb(0x40, (divisor >> 8) & 0xff);

  sti(); // Interrupts were disabled in "boot.asm".
  while(1) {
    asm volatile ("hlt");
  }
}
