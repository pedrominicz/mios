#include "terminal.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

static const uint16_t port = 0x03f8;
static uint8_t* const terminal = (uint8_t*)0xf00b8000;
static volatile size_t cursor_x = 0, cursor_y = 0;

void init_terminal(void) {
  // Initialize serial port 1.
  outb(port + 1, 0x00); // Disable all interrupts.
  outb(port + 3, 0x80); // Set divisor latch bit on line control.
  const uint32_t baud = 115200;
  const uint32_t divisor = 115200 / baud;
  outb(port + 0, divisor & 0xff);
  outb(port + 1, (divisor >> 8) & 0xff);
  outb(port + 3, 0x03); // 8 bits, one stop bit, no parity.
  outb(port + 2, 0xc7); // Enable FIFO, clear them, with 14-byte threshold.
  // Set data terminal ready and request to send bits on modem control
  // register.
  outb(port + 4, 0x03);

  // Clear terminal.
  for(size_t i = 0; i < 80 * 25 * 2; i += 2) {
    terminal[i] = ' ';
  }
  cursor_x = cursor_y = 0;

  terminal_putchar('m');
  terminal_putchar('i');
  terminal_putchar('o');
  terminal_putchar('s');
  terminal_putchar('\n');
}

static void terminal_scroll(void) {
  // Copy every character to the line above.
  for(size_t y = 1; y < 25; ++y) {
    for(size_t x = 0; x < 80; ++x) {
      terminal[2 * ((y - 1) * 80 + x)] = terminal[2 * (y * 80 + x)];
    }
  }

  // Clear last line.
  for(size_t x = 0; x < 80; ++x) {
    terminal[2 * (24 * 80 + x)] = ' ';
  }
}

void terminal_putchar(const char c) {
  // Wait until transmitter holding register is empty.
  while(!(inb(port + 5) & 0x20)) { }
  // Write character to serial port 1.
  outb(port, c);

  if(cursor_y >= 25) {
    terminal_scroll();
    cursor_y = 24;
  }

  if(c == '\n') {
    cursor_x = 0;
    cursor_y += 1;
  } else if (c == '\t') {
    cursor_x -= cursor_x % 8;
    cursor_x += 8;
  } else {
    terminal[2 * (cursor_y * 80 + cursor_x++)] = c;
  }

  if(cursor_x >= 80) {
    cursor_x = 0;
    cursor_y += 1;
  }
}
