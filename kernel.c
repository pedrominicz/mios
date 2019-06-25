#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stddef.h>
#include <stdint.h>

static uint32_t count = 0;

// Programmable interval timer (PIT) interrupt handler.
static void pit_interrupt_handler(const InterruptFrame* const frame) {
  (void)frame;
  terminal_putchar('.');
  if(++count > 30) {
    asm volatile ("int 3");
  }
}

void mios_init(void) {
  init_interrupt();
  init_terminal();

  terminal_print("Hello many layers of indirection world!\n");
  uintmax_t all_digits = 0x01234567;
  all_digits <<= 32;
  all_digits |= 0x89abcdef;
  terminal_print_hex(all_digits);

  // Start the programmable interval timer (PIT).
  const uint32_t frequency = 50;
  const uint32_t divisor = 1193180 / frequency;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xff);
  outb(0x40, (divisor >> 8) & 0xff);

  register_interrupt_handler(0x20, pit_interrupt_handler);

  sti(); // Interrupts were disabled in "boot.asm".
  while(1) {
    asm volatile ("hlt");
  }
}
