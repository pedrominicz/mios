#include "interrupt.h"
#include "memory.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Kernel context registers. Note that `esp` is saved as the address of the
// `Context` itself.
typedef struct Context {
  // Pushed by `switch_context`.
  uint32_t edi;
  uint32_t esi;
  uint32_t ebx;
  uint32_t ebp;
  // Pushed by `call` instruction.
  uint32_t eip;
} Context;

typedef struct Process {
  bool present;
  InterruptFrame* frame;
  Context* context;
  void* kernel_stack;
} Process;

typedef struct Core {
  Process* process;
  Context* scheduler;
  uint64_t gdt[6];
  uint32_t tss[26];
} Core;

Context* init_context;
Context* loop_context;

Core* core;
Process process[16];

// Initialize global descriptor table (GDT).
void init_gdt(Core* const core) {
  core->gdt[0] = 0x0000000000000000; // Null segment.
  core->gdt[1] = 0x00cf9a000000ffff; // Kernel code segment.
  core->gdt[2] = 0x00cf92000000ffff; // Kernel data segment.
  core->gdt[3] = 0x00cffa000000ffff; // User mode code segment.
  core->gdt[4] = 0x00cff2000000ffff; // User mode data segment.
  core->gdt[5] = 0x0000e90000000068; // Task state segment (TSS).

  // Task state segment base address (bits 0-23).
  core->gdt[5] |= ((uint64_t)(uintptr_t)core->tss & 0x00ffffff) << 16;
  // Task state segment base address (bits 24-31).
  core->gdt[5] |= ((uint64_t)(uintptr_t)core->tss & 0xff000000) << 32;

  volatile uint64_t gdt_descriptor = 0;
  gdt_descriptor |= sizeof(core->gdt) - 1;
  gdt_descriptor |= (uint64_t)(uintptr_t)core->gdt << 16;

  asm volatile ("lgdt (%0);" :: "r"(&gdt_descriptor));

  core->tss[2] = KERNEL_DATA_SEGMENT; // Kernel stack segment.
  // Make segment registers accessible from privilege level-3; i.e. user mode.
  core->tss[18] = KERNEL_DATA_SEGMENT | 3; // Extra segment.
  core->tss[19] = KERNEL_CODE_SEGMENT | 3; // Code segment.
  core->tss[20] = KERNEL_DATA_SEGMENT | 3; // Stack segment.
  core->tss[21] = KERNEL_DATA_SEGMENT | 3; // Data segment.
  core->tss[22] = KERNEL_DATA_SEGMENT | 3; // Extra segment #2 (`fs`).
  core->tss[23] = KERNEL_DATA_SEGMENT | 3; // Extra segment #3 (`gs`).
  // Setting I/O map base address beyond the TSS limit and setting I/O
  // privilege level to 0 in `eflags` disables I/O instructions in user mode.
  core->tss[25] = 0xffff0000; // I/O map base address.
}

extern void switch_context(Context** old, Context* new); // Defined in "switch.S".
extern void interrupt_return(void); // Defined in "interrupts.S".

void init_user_page_directory(uint32_t* const user_page_directory) {
  for(size_t i = 0; i < 1024; ++i) {
    const uint32_t page_address = (i << 22) - KERNEL_OFFSET;
    user_page_directory[i] = 0;
    user_page_directory[i] |= page_address & 0xffc00000;
    user_page_directory[i] |= 0x87;
  }
}

static void loop0(void) {
  while(1) {
    for(size_t i = 0; i < 10000000; ++i) { }
    syscall("Loop zero.\n");
  }
}

static void loop1(void) {
  while(1) {
    for(size_t i = 0; i < 10000000; ++i) { }
    syscall("Loop one.\n");
  }
}

void set_kernel_stack(Core* const core, void* kernel_stack) {
  core->gdt[5] &= ~0x0000020000000000; // Clear busy flag.
  core->tss[1] = (uintptr_t)kernel_stack;
  asm volatile ("ltr %w0" :: "r"(TASK_STATE_SEGMENT));
}

void make_process(size_t i, void (*function)(void)) {
  uint8_t* stack = (uint8_t*)malloc_page() + 4096;

  stack -= sizeof(InterruptFrame);
  process[i].frame = (InterruptFrame*)stack;
  process[i].frame->ds = USER_DATA_SEGMENT;
  process[i].frame->es = process[i].frame->ds;
  process[i].frame->ss = process[i].frame->ds;
  process[i].frame->cs = USER_CODE_SEGMENT;
  process[i].frame->eip = (uintptr_t)function;
  process[i].frame->esp = (uintptr_t)malloc_page() + 4096;

  stack -= sizeof(Context);
  process[i].context = (Context*)stack;
  process[i].context->eip = (uintptr_t)interrupt_return;

  process[i].kernel_stack = (void*)((uintptr_t)malloc_page() + 4096);

  process[i].present = true;
}

void hack(void) {
  switch_context(&core->process->context, core->scheduler);
}

void init(void) {
  Core init_core = {0};
  core = &init_core;

  init_kernel_page_directory();
  init_kernel_malloc();
  init_gdt(core); // Global descriptor table (GDT).
  init_idt(); // Interrupt descriptor table (IDT).
  init_pic();

  uint32_t* user_page_directory = malloc_page();
  init_user_page_directory(user_page_directory);

  make_process(0, loop0);
  make_process(1, loop1);

  while(1) {
    for(size_t i = 0; i < 16; ++i) {
      if(!process[i].present) continue;

      core->process = &process[i];

      switch_page_directory(user_page_directory);
      set_kernel_stack(core, core->process->kernel_stack);
      switch_context(&core->scheduler, core->process->context);
      switch_page_directory(kernel_page_directory);
    }
  }
}
