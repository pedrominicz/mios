#include "idt.h"
#include "gdt.h"

#include <stddef.h>
#include <stdint.h>

extern uintptr_t traps[256]; // Defined in "traps.S".
static uint64_t idt[256];

static inline uint64_t make_interrupt_gate(const uintptr_t interrupt) {
  const uint64_t interrupt_low = interrupt;
  const uint64_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= interrupt_high << 48;
  idt_entry |= KERNEL_CODE_SELECTOR << 16;
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0x8e << 40;
  return idt_entry;
}

void init_idt(void) {
  for(size_t i = 0; traps[i]; ++i) {
    idt[i] = make_interrupt_gate(traps[i]);
  }

  volatile uint64_t idt_descriptor = 0;
  idt_descriptor |= sizeof(idt) - 1;
  idt_descriptor |= (uint64_t)(uintptr_t)idt << 16;

  asm volatile ("lidt (%0)" :: "r"(&idt_descriptor));
}
