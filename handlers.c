#include "interrupt.h"
#include "terminal.h"
#include "x86.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static volatile bool breakpoint_continue = false;

static char* interrupt_name[256] = {
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

  [0x10] = "x87 fpu floating-point error",
  [0x11] = "alignment check",
  [0x12] = "machine check",
  [0x13] = "simd floating-point exception",
  [0x14] = "virtualization exception",
};

static void invalid_tss_print_error(uint32_t error_code);
static void segment_not_present_print_error(uint32_t error_code);
static void stack_segment_fault_print_error(uint32_t error_code);
static void general_protection_print_error(uint32_t error_code);
static void page_fault_print_error(uint32_t error_code);

void (*interrupt_print_error[256])(uint32_t error_code) = {
  [0x0a] = invalid_tss_print_error,
  [0x0b] = segment_not_present_print_error,
  [0x0c] = stack_segment_fault_print_error,
  [0x0d] = general_protection_print_error,
  [0x0e] = page_fault_print_error,
};

static void invalid_tss_print_error(uint32_t error_code) {
  (void)error_code;
  terminal_putchar('\n');
}

static void segment_not_present_print_error(uint32_t error_code) {
  (void)error_code;
  terminal_putchar('\n');
}

static void stack_segment_fault_print_error(uint32_t error_code) {
  (void)error_code;
  terminal_putchar('\n');
}

static void general_protection_print_error(uint32_t error_code) {
  (void)error_code;
  terminal_putchar('\n');
}

static void page_fault_print_error(uint32_t error_code) {
  terminal_print("\n\n");

  terminal_print(error_code & 0x00000001 ?
      "\tThe fault was caused by a page-level protection violation (bit 0 set).\n" :
      "\tThe fault was caused by a non-present page (bit 0 clear).\n");

  terminal_print(error_code & 0x00000002 ?
      "\tThe access causing the fault was a write (bit 1 set).\n" :
      "\tThe access causing the fault was a read (bit 1 clear).\n");

  terminal_print(error_code & 0x00000004 ?
      "\tA user-mode access caused the fault (bit 2 set).\n" :
      "\tA supervisor-mode access caused the fault (bit 2 clear).\n");

  if(error_code & 0x00000008) {
    terminal_print("\tThe fault was caused by a reserved bit set to 1 in some paging-structure entry (bit 3 set).\n");
  }

  if(error_code & 0x00000010) {
    terminal_print("\tThe fault was caused by an instruction fetch (bit 4 set).\n");
  }
}


void interrupt_handler(InterruptFrame* frame) {
  terminal_print("Interrupt 0x");
  terminal_print_hex8(frame->interrupt_number);

  if(interrupt_name[frame->interrupt_number]) {
    terminal_print(" (");
    terminal_print(interrupt_name[frame->interrupt_number]);
    terminal_putchar(')');
  }

  terminal_putchar('.');

  if(interrupt_print_error[frame->interrupt_number]) {
    terminal_print(" Error code 0x");
    terminal_print_hex32(frame->interrupt_error_code);
    terminal_putchar('.');
    interrupt_print_error[frame->interrupt_number](frame->interrupt_error_code);
  } else {
    terminal_putchar('\n');
  }

  terminal_putchar('\n');

  terminal_print("    eax 0x"); terminal_print_hex32(frame->eax);
  terminal_print("    ecx 0x"); terminal_print_hex32(frame->ecx);
  terminal_print("    edx 0x"); terminal_print_hex32(frame->edx);
  terminal_print("    ebx 0x"); terminal_print_hex32(frame->ebx);
  terminal_putchar('\n');

  terminal_print("   _esp 0x"); terminal_print_hex32(frame->_esp);
  terminal_print("    ebp 0x"); terminal_print_hex32(frame->ebp);
  terminal_print("    esi 0x"); terminal_print_hex32(frame->esi);
  terminal_print("    edi 0x"); terminal_print_hex32(frame->edi);
  terminal_putchar('\n');

  terminal_print("    eip 0x"); terminal_print_hex32(frame->eip);
  terminal_print(" eflags 0x"); terminal_print_hex32(frame->eflags);
  terminal_print("     cs 0x"); terminal_print_hex16(frame->cs);
  terminal_putchar('\n');

  terminal_print("     ds 0x"); terminal_print_hex16(frame->ds);
  terminal_print("         es 0x"); terminal_print_hex16(frame->es);
  terminal_print("         fs 0x"); terminal_print_hex16(frame->fs);
  terminal_print("         gs 0x"); terminal_print_hex16(frame->gs);
  terminal_putchar('\n');

  while(1) {
    asm volatile ("cli; hlt");
  }
}

void breakpoint_handler(InterruptFrame* frame) {
  terminal_print("Breakpoint at \"");
  terminal_print((const char*)frame->eax);
  terminal_print("\" line #");
  terminal_print_decimal(frame->ebx);
  terminal_print(". Press ENTER to continue...\n");

  asm volatile ("sti");
  breakpoint_continue = false;
  while(!breakpoint_continue) {
    asm volatile ("hlt");
  }
}

void keyboard_handler(InterruptFrame* frame) {
  (void)frame;
  if(inb(0x60) == 0x9c) { // Enter key released.
    breakpoint_continue = true;
  }
}

void syscall_handler(InterruptFrame* frame) {
  terminal_print("Syscall 0x");
  terminal_print_hex(frame->eax);
  terminal_print(".\n");
}
