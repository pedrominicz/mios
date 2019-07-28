#include "interrupt.h"
#include "memory.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern const uintptr_t interrupts[]; // Defined in "interrupts.S".
static uint64_t idt[SYSCALL_INTERRUPT + 2];

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

static const char* const interrupt_name[] = {
  [0x00] = "divide error (`div` and `idiv`)",
  [0x01] = "debug exception",
  [0x02] = "non maskable interrupt (NMI)",
  [0x03] = "breakpoint (`int $3`)",
  [0x04] = "overflow (`into`)",
  [0x05] = "bound range exception (`bound`)",
  [0x06] = "invalid opcode (`ud2`)",
  [0x07] = "device not available (`wait` and `fwait`)",
  [0x08] = "double fault",
  [0x09] = "coprocessor segment overrun (floating-point instructions)",
  [0x0a] = "invalid task statement segment (TTS)",
  [0x0b] = "segment not present",
  [0x0c] = "stack segment fault",
  [0x0d] = "general protection fault",
  [0x0e] = "page fault",

  [0x10] = "x87 FPU floating-point error",
  [0x11] = "alignment check",
  [0x12] = "machine check",
  [0x13] = "simd floating-point exception",
  [0x14] = "virtualization exception",
};

static void invalid_tss_print_error(const uint32_t error_code);
static void segment_not_present_print_error(const uint32_t error_code);
static void stack_segment_fault_print_error(const uint32_t error_code);
static void general_protection_print_error(const uint32_t error_code);
static void page_fault_print_error(const uint32_t error_code);

static void (* const interrupt_print_error[256])(const uint32_t error_code) = {
  [0x0a] = invalid_tss_print_error,
  [0x0b] = segment_not_present_print_error,
  [0x0c] = stack_segment_fault_print_error,
  [0x0d] = general_protection_print_error,
  [0x0e] = page_fault_print_error,
};

static void invalid_tss_print_error(const uint32_t error_code) {
  (void)error_code;
}

static void segment_not_present_print_error(const uint32_t error_code) {
  (void)error_code;
}

static void stack_segment_fault_print_error(const uint32_t error_code) {
  (void)error_code;
}

static void general_protection_print_error(const uint32_t error_code) {
  if(error_code) {
    const bool e0 = error_code & 1;
    const bool e1 = error_code & 2;
    const bool e2 = error_code & 4;

    if(e0) {
      printf("\tThe exception occurred during delivery of an event external to the program (bit 0 set).\n");
    }

    printf("\tSegment selector index (%s): 0x%04lx\n\n",
        e1 ? "IDT" : e2 ? "LDT" : "GDT",
        error_code & ~3);
  }
}

static void page_fault_print_error(const uint32_t error_code) {
  const bool e0 = error_code & 1;
  const bool e1 = error_code & 2;
  const bool e2 = error_code & 4;
  const bool e3 = error_code & 8;
  const bool e4 = error_code & 16;

  printf("\tThe fault was caused by a %s.\n"
      "\tThe access causing the fault was a %s.\n"
      "\tA %s-mode access caused the fault.\n",
      e0 ? "protection violation" : "non-present page",
      e1 ? "write" : "read",
      e2 ? "user" : "supervisor");

  if(e3) {
    printf("\tThe fault was caused by a reserved bit set to 1.\n");
  }

  if(e4) {
    printf("\tThe fault was caused by an instruction fetch.\n");
  }

  putchar('\n');
}

void hack(void); // Defined in "init.c".

void interrupt_handler(const InterruptFrame* const frame) {
  if(frame->interrupt_number == SYSCALL_INTERRUPT) {
    printf("%s", (char*)frame->eax);
    hack();
    return;
  }

  // Breakpoint interrupt used as an easy way to print all registers.
  if(frame->interrupt_number != 3) {
    const char* const name = interrupt_name[frame->interrupt_number] ?
      interrupt_name[frame->interrupt_number] :
      "unknown interrupt";

    printf("Interrupt 0x%02lx (%s). Error code 0x%08lx.\n\n",
        frame->interrupt_number, name, frame->interrupt_error_code);

    if(interrupt_print_error[frame->interrupt_number]) {
      interrupt_print_error[frame->interrupt_number](frame->interrupt_error_code);
    }
  }

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

  if(frame->interrupt_number == 3) return;

  while(1) {
    asm volatile ("cli; hlt");
  }
}
