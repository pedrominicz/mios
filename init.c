#include "interrupt.h"
#include "memory.h"
#include "util.h"

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

Context* init_context;
Context* loop_context;

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

static void loop(void) {
  Context context = {0};
  loop_context = &context;

  syscall();

  while(1) { }
}

void init(void) {
  init_kernel_page_directory();
  init_kernel_malloc();
  init_gdt(); // Global descriptor table (GDT).
  init_idt(); // Interrupt descriptor table (IDT).

  uint32_t* user_page_directory = malloc_page();
  init_user_page_directory(user_page_directory);
  switch_page_directory(user_page_directory);

  uint8_t* stack = (uint8_t*)malloc_page() + 4096;

  stack -= sizeof(InterruptFrame);
  InterruptFrame* const frame = (InterruptFrame*)stack;
  frame->ds = USER_DATA_SEGMENT;
  frame->es = frame->ds;
  frame->ss = frame->ds;
  frame->cs = USER_CODE_SEGMENT;
  frame->eip = (uintptr_t)loop;
  frame->esp = (uintptr_t)malloc_page() + 4096;

  stack -= sizeof(Context);
  loop_context = (Context*)stack;
  loop_context->eip = (uintptr_t)interrupt_return;

  Context context = {0};
  init_context = &context;

  set_kernel_stack((void*)((uintptr_t)malloc_page() + 4096));

  switch_context(&init_context, loop_context);

  while(1) {
    asm volatile ("hlt");
  }
}
