#include "interrupt.h"
#include "memory.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern const uintptr_t interrupts[]; // Defined in "interrupts.S".
static uint64_t idt[SYSCALL_INTERRUPT + 1];

static inline uint64_t make_idt_entry(const uintptr_t interrupt) {
  const uint16_t interrupt_low = interrupt;
  const uint16_t interrupt_high = interrupt >> 16;

  uint64_t idt_entry = 0;
  idt_entry |= interrupt_low;
  idt_entry |= KERNEL_CODE_SEGMENT << 16;
  // Interrupt type and attributes.
  idt_entry |= (uint64_t)0x8e << 40;
  idt_entry |= (uint64_t)interrupt_high << 48;
  return idt_entry;
}

// Initialize interrupt descriptor table (IDT).
void init_idt(void) {
  for(size_t i = 0; interrupts[i]; ++i) {
    idt[i] = make_idt_entry(interrupts[i]);
  }

  // Make syscall accessible from user mode.
  idt[SYSCALL_INTERRUPT] |= (uint64_t)0x60 << 40;

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
  // Set interrupt masks to ignore everything.
  outb(0x21, 0xff);
  outb(0xa1, 0xff);
}

void interrupt_handler(const InterruptFrame* const frame) {
  if(frame->interrupt_number == SYSCALL_INTERRUPT) {
    printf("Syscall.\n");
    return;
  }

  printf("Interrupt 0x%02lx (error code 0x%08lx).\n\n",
      frame->interrupt_number, frame->interrupt_error_code);

  printf("    eax 0x%08lx    ecx 0x%08lx    edx 0x%08lx    ebx 0x%08lx\n"
         "   _esp 0x%08lx    ebp 0x%08lx    esi 0x%08lx    edi 0x%08lx\n"
         "    eip 0x%08lx eflags 0x%08lx     cs 0x%04lx\n"
         "     ds 0x%04lx         es 0x%04lx         fs 0x%04lx         gs 0x%04lx\n"
         "    esp 0x%08lx     ss 0x%04lx\n",
      frame->eax, frame->ecx, frame->edx, frame->ebx,
      frame->_esp, frame->ebp, frame->esi, frame->edi,
      frame->eip, frame->eflags, frame->cs,
      frame->ds, frame->es, frame->fs, frame->gs,
      frame->esp, frame->ss);

  while(1) {
    asm volatile ("cli; hlt");
  }
}
