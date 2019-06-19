#include <stddef.h>
#include <stdint.h>

uint8_t array[80 * 25 * 2];

void mios_init(void) {
  static uint8_t* const terminal = (uint8_t*)0xb8000;

  for(size_t i = 0; i < 80 * 25 * 2; i += 2) {
    // Try commenting line 170 in boot.asm.
    terminal[i] = array[i] + ' ';
  }

  terminal[0] = 'H';
  terminal[2] = 'e';
  terminal[4] = 'l';
  terminal[6] = 'l';
  terminal[8] = 'o';
  terminal[10] = ' ';
  terminal[12] = 'k';
  terminal[14] = 'e';
  terminal[16] = 'r';
  terminal[18] = 'n';
  terminal[20] = 'e';
  terminal[22] = 'l';
  terminal[24] = ' ';
  terminal[26] = 'w';
  terminal[28] = 'o';
  terminal[30] = 'r';
  terminal[32] = 'l';
  terminal[34] = 'd';
  terminal[36] = '!';

  while(1) {
    asm volatile ("cli");
    asm volatile ("hlt");
  }
}
