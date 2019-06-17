#include <stdint.h>

__attribute__((noreturn)) void mios_init(void) {
  uint8_t* terminal = (uint8_t*)0xb8000;
  terminal[0] = 'x';

  while(1);
  __builtin_unreachable();
}
