#ifndef MIOS_INTERRUPT_H
#define MIOS_INTERRUPT_H

#define SYSCALL_INTERRUPT 0x30 // Same as in "interrupts.S".

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct InterruptFrame {
  // Pushed by interrupt entry.
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t _esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;
  uint32_t interrupt_number;
  // Pushed by the CPU or by interrupt entry.
  uint32_t interrupt_error_code;
  // Pushed by the CPU.
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  // Pushed by the CPU only when crossing rings.
  uint32_t esp;
  uint32_t ss;
} InterruptFrame;

// Initialize interrupt descriptor table (IDT).
void init_idt(void);
// Initialize programmable interrupt controller (PIC).
void init_pic(void);

#endif // __ASSEMBLER__

#endif // MIOS_INTERRUPT_H
