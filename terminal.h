#ifndef MIOS_TERMINAL_H
#define MIOS_TERMINAL_H

#include "x86.h"

#include <stddef.h>
#include <stdint.h>

static uint8_t* const terminal = (uint8_t*)0xb8000;
static volatile size_t cursor_x = 0, cursor_y = 0;

static inline void terminal_clear(void) {
  for(size_t i = 0; i < 80 * 25 * 2; i += 2) {
    terminal[i] = ' ';
  }

  cursor_x = cursor_y = 0;
  // 0x03d4 selects which register will be used by 0x03b5.
  outb(0x03d4, 14); // High bits of cursor position.
  outb(0x03d5, 0);
  outb(0x03d4, 15); // Low bits of cursor position.
  outb(0x03d5, 0);
}

static inline void terminal_putchar(const char c) {
  // Quit if printing outside the screen. Eventually we will support scrolling.
  if(cursor_y > 25) return;

  if(c == '\n') {
    cursor_x = 0;
    cursor_y += 1;
    return;
  }

  terminal[2 * (cursor_y * 80 + cursor_x)] = c;

  if(++cursor_x > 80) {
    cursor_x = 0;
    cursor_y += 1;
  }

  const uint16_t cursor_position = cursor_y * 80 + cursor_x;
  // 0x03d4 selects which register will be used by 0x03b5.
  outb(0x03d4, 14); // High bits of cursor position.
  outb(0x03d5, cursor_position >> 8);
  outb(0x03d4, 15); // Low bits of cursor position.
  outb(0x03d5, cursor_position);
}

static inline void terminal_print(const char* s) {
  for(char c = *s; c != 0; c = *(++s)) {
    terminal_putchar(c);
  }
}

#endif // MIOS_TERMINAL_H
