#include "interrupt.h"
#include "memory.h"
#include "x86.h"

#include <stddef.h>
#include <stdint.h>

extern uintptr_t interrupts[256]; // Defined in "interrupts.S".
static uint64_t idt[256];

static inline uint64_t make_interrupt_gate(const uintptr_t interrupt) {
  const uint16_t interrupt_low = interrupt;
  const uint16_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= KERNEL_CODE_SELECTOR << 16;
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0x8e << 40;
  idt_entry |= (uint64_t)interrupt_high << 48;
  return idt_entry;
}

// Initialize interrupt descriptor table (IDT).
void init_idt(void) {
  for(size_t i = 0; i < 256; ++i) {
    if(interrupts[i]) idt[i] = make_interrupt_gate(interrupts[i]);
  }

  // Make breakpoints and syscalls accessible from user mode.
  if(idt[0x03]) idt[0x03] |= (uint64_t)0x60 << 40;
  if(idt[0x80]) idt[0x80] |= (uint64_t)0x60 << 40;

  volatile uint64_t idt_descriptor = 0;
  idt_descriptor |= sizeof(idt) - 1;
  idt_descriptor |= (uint64_t)(uintptr_t)idt << 16;

  asm volatile ("lidt (%0)" :: "r"(&idt_descriptor));
}

// Initialize programmable interrupt controller (PIC).
void init_pic(void) {
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
  // Set interrupt masks to ignore everything except keyboard interrupts.
  outb(0x21, 0xfd);
  outb(0xa1, 0xff);
}
