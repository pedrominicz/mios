#include "interrupt.h"
#include "memory.h"
#include "syscall.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

void init(void) {
  init_terminal();
  init_idt();

  init_kernel_page_directory();
  init_kernel_malloc();

  while(1) {
    asm volatile ("hlt");
  }
}
