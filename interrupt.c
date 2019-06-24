#include "interrupt.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Interrupt {
  // `pusha`
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  // `push gs`
  uint16_t gs;
  uint16_t gs_padding;
  // `push fs`
  uint16_t fs;
  uint16_t fs_padding;
  // `push es`
  uint16_t es;
  uint16_t es_padding;
  // `push ds`
  uint16_t ds;
  uint16_t ds_padding;
  // `push dword %1`
  uint32_t number;
  // `push dword 0` or pushed by the CPU.
  uint32_t error_code;
  // Pushed by the CPU.
  uint32_t eip;
  uint16_t cs;
  uint16_t cs_padding;
  uint16_t eflags;
} Interrupt;

extern uintptr_t interrupts[256]; // Defined in "interrupt-stubs.asm".
static uint64_t idt[256];

static inline uint64_t make_interrupt_gate(const uintptr_t interrupt) {
  const uint64_t interrupt_low = interrupt;
  const uint64_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= interrupt_high << 48;
  idt_entry |= 0x0008 << 16; // Kernel code selector.
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0b10001110 << 40;
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

void init_interrupt(void) {
  init_idt();
}

void interrupt_handle(const Interrupt* const interrupt) {
  (void)interrupt;
  terminal_print("interrupt\n");
}
