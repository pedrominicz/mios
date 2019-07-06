#include "gdt.h"
#include "interrupt.h"
#include "memory.h"
#include "syscall.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

void init(void) {
  init_terminal();
  init_memory();
  init_gdt();
  init_idt();
  init_pic();

  while(1) {
    asm volatile ("hlt");
  }
}
