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

static void loop(void) {
  Context context = {0};
  loop_context = &context;

  size_t count = 0;
  while(1) {
    for(size_t i = 0; i < 5000000; ++i) { }
    printf("%zu", ++count);
    switch_context(&loop_context, init_context);
  }
}

void init(void) {
  init_kernel_page_directory();
  init_kernel_malloc();
  init_gdt(); // Global descriptor table (GDT).
  init_idt(); // Interrupt descriptor table (IDT).

  uint8_t* stack = malloc_page();
  stack += 4096 - sizeof(Context);
  loop_context = (Context*)stack;
  loop_context->eip = (uint32_t)loop;

  Context context = {0};
  init_context = &context;

  while(1) {
    switch_context(&init_context, loop_context);
    putchar('\n');
  }

  while(1) {
    asm volatile ("hlt");
  }
}
