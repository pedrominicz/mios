#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern uintptr_t interrupts[256]; // Defined in "interrupt-stubs.asm".
static uint64_t idt[256];
static InterruptHandler handler[256];

// Initialize programmable interrupt controller (PIC).
static inline void init_pic(void) {
  // Restart PICs.
  outb(0x20, 0x11); // Master PIC command port.
  outb(0xa0, 0x11); // Slave PIC command port.
  // Configure interrupt vector offset.
  outb(0x21, 0x20); // Master PIC data port.
  outb(0xa1, 0x28); // Slave PIC data port.
  // Connect both PICs.
  outb(0x21, 0x04); // Tell master slave is at interrupt request line 2.
  outb(0xa1, 0x02); // Tell slave its cascade identity.
  // XXX
  outb(0x21, 0x01);
  outb(0xa1, 0x01);
  // Set interrupt masks. Ignore everything except the programmable interval
  // timer (PIT).
  outb(0x21, 0xfe);
  outb(0xa1, 0xff);
}

static inline uint64_t make_interrupt_gate(const uintptr_t interrupt) {
  const uint64_t interrupt_low = interrupt;
  const uint64_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= interrupt_high << 48;
  idt_entry |= 0x0008 << 16; // Kernel code selector.
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0x8e << 40;
  return idt_entry;
}

static inline void init_idt(void) {
  for(size_t i = 0; interrupts[i]; ++i) {
    idt[i] = make_interrupt_gate(interrupts[i]);
  }

  volatile uint64_t idt_descriptor = 0;
  idt_descriptor |= sizeof(idt) - 1;
  idt_descriptor |= (uint64_t)(uint32_t)idt << 16;

  asm volatile ("lidt [%0]" :: "r"(&idt_descriptor));
}

void default_interrupt_handler(const InterruptFrame* const frame) {
  terminal_print("Unhandled interrupt (0x");
  terminal_print_hex(frame->interrupt_number);
  terminal_print(")! Hanging...\n");
  hang();
}

void init_interrupt(void) {
  init_pic();
  init_idt();

  for(size_t i = 0; i < 256; ++i) {
    handler[i] = default_interrupt_handler;
  }
}

void register_interrupt_handler(uint8_t interrupt_number, InterruptHandler function) {
  handler[interrupt_number] = function;
}

void interrupt_handler(const InterruptFrame* const frame) {
  handler[frame->interrupt_number](frame);
}
