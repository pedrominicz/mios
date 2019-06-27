#include "terminal.h"
#include "x86.h"

#include <stddef.h>
#include <stdint.h>

static const uint16_t port1 = 0x03f8;
static uint8_t* const terminal = (uint8_t*)0xb8000;
static volatile size_t cursor_x = 0, cursor_y = 0;

void init_terminal(void) {
  // Initialize serial port 1.
  outb(port1 + 1, 0x00); // Disable all interrupts.
  outb(port1 + 3, 0x80); // Set divisor latch bit on line control.
  const uint32_t baud = 115200;
  const uint32_t divisor = 115200 / baud;
  outb(port1 + 0, divisor & 0xff);
  outb(port1 + 1, (divisor >> 8) & 0xff);
  outb(port1 + 3, 0x03); // 8 bits, one stop bit, no parity.
  outb(port1 + 2, 0xc7); // Enable FIFO, clear them, with 14-byte threshold.
  // Set data terminal ready and request to send bits on modem control
  // register.
  outb(port1 + 4, 0x03);

  terminal_clear();
}

void terminal_clear(void) {
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

void terminal_print_hex(uintmax_t n) {
  static const char hex_digit[16] = "0123456789abcdef";
  static char s[sizeof(uintmax_t) * 2 + 1] = {0};
  size_t i = sizeof(uintmax_t) * 2;
  for(; n; n /= 16) {
    s[--i] = hex_digit[n & 0xf];
  }
  terminal_print(s + i);
}

void terminal_putchar(const char c) {
  // Quit if printing outside the screen. Eventually we will support scrolling.
  if(cursor_y > 25) return;

  // Wait until transmitter holding register is empty.
  while(!(inb(port1 + 5) & 0x20)) { }
  // Write character to serial port 1.
  outb(port1, c);

  if(c == '\n') {
    cursor_x = 0;
    cursor_y += 1;
  } else {
    terminal[2 * (cursor_y * 80 + cursor_x)] = c;
    if(++cursor_x >= 80) {
      cursor_x = 0;
      cursor_y += 1;
    }
  }

  const uint16_t cursor_position = cursor_y * 80 + cursor_x;
  // 0x03d4 selects which register will be used by 0x03b5.
  outb(0x03d4, 14); // High bits of cursor position.
  outb(0x03d5, cursor_position >> 8);
  outb(0x03d4, 15); // Low bits of cursor position.
  outb(0x03d5, cursor_position);
}
