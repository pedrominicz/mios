#include "memory.h"
#include "util.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const uint16_t port = 0x03f8;
// Static objects must be initialized with constant expressions. Only literals
// can be `const` so `physical_to_virtual(0xb8000)` cannot be used.
static uint8_t* const terminal = (uint8_t*)(0xb8000 + KERNEL_OFFSET);
static volatile size_t cursor_x = 0, cursor_y = 0;

static void init_terminal(void) {
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
}

static void terminal_scroll(void) {
  // Copy every character to the line above.
  for(size_t y = 1; y < 25; ++y) {
    for(size_t x = 0; x < 80; ++x) {
      terminal[2 * ((y - 1) * 80 + x)] = terminal[2 * (y * 80 + x)];
    }
  }

  // Clear last line.
  for(size_t i = 0; i < 80; ++i) {
    terminal[2 * (24 * 80 + i)] = ' ';
  }
}

void _putchar(char c) {
  static bool first = true;
  if(first) {
    first = false;
    init_terminal();
  }

  // Wait until transmitter holding register is empty.
  while(!(inb(port + 5) & 0x20)) { }
  // Write character to serial port 1.
  outb(port, c);

  if(cursor_x >= 80) {
    cursor_x = 0;
    cursor_y += 1;
  }

  if(cursor_y >= 25) {
    terminal_scroll();
    cursor_y = 24;
  }

  switch(c) {
  case '\n':
    cursor_y += 1;
    // Fallthrough.
  case '\r':
    cursor_x = 0;
    break;
  case '\t':
    cursor_x -= cursor_x % 8;
    cursor_x += 8;
    break;
  default:
    terminal[2 * (cursor_y * 80 + cursor_x++)] = c;
    break;
  }
}

static inline void print_string(const char* s) {
  if(!s) return;
  for(char c = *s; c != 0; c = *(++s)) {
    _putchar(c);
  }
}

static inline void print_unsigned(uint64_t x, bool hex, size_t pad, bool pad_zero) {
  static const char digits[16] = "0123456789abcdef";
  char s[21] = {0};
  int divisor = hex ? 16 : 10;

  size_t i = 20;
  do {
    s[--i] = digits[x % divisor];
    x /= divisor;
  } while(x);

  for(; pad > 20 - i; --pad) {
    _putchar(pad_zero ? '0' : ' ');
  }

  print_string(s + i);
}

void _printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  for(; *format; ++format) {
    if(*format != '%') {
      _putchar(*format);
      continue;
    }

    const char* start = format;
    ++format;

    // Flags field.
    bool pad_zero = false;
    if(*format == '0') {
      pad_zero = true;
      ++format;
    }

    // Width field.
    size_t pad = 0;
    for(; *format >= '0' && *format <= '9'; ++format) {
      pad *= 10;
      pad += *format - '0';
    }

    // Length field.
    size_t size = sizeof(int);
    if(*format == 'l') {
      size = sizeof(long);
      ++format;
      if(*format == 'l') {
        size = sizeof(long long);
        ++format;
      }
    } else if(*format == 'z') {
      size = sizeof(size_t);
      ++format;
    }

    // Type field and print.
    bool hex = false;
    switch(*format) {
    case 'p': // Pointer.
      hex = true;
      pad = sizeof(uintptr_t) * 2;
      pad_zero = true;
      print_unsigned(va_arg(ap, uintptr_t), hex, pad, pad_zero);
      break;
    case 's': // String.
      print_string(va_arg(ap, char*));
      break;
    case 'x': // Unsigned hexadecimal number.
      hex = true;
      // Fallthrough.
    case 'u': // Unsigned decimal number.
      if(size == sizeof(int)) {
        print_unsigned(va_arg(ap, unsigned int), hex, pad, pad_zero);
      } else if(size == sizeof(long)) {
        print_unsigned(va_arg(ap, unsigned long), hex, pad, pad_zero);
      } else if(size == sizeof(long long)) {
        print_unsigned(va_arg(ap, unsigned long long), hex, pad, pad_zero);
      } else if(size == sizeof(size_t)) {
        print_unsigned(va_arg(ap, size_t), hex, pad, pad_zero);
      }
      break;
    case '%': // Percent sign.
      if(start + 1 == format) {
        _putchar('%');
        break;
      }
      // Fallthrough.
    default: // Unknown format sequence.
      for(; start <= format; ++start) {
        _putchar(*start);
      }
      break;
    }
  }

  va_end(ap);
}
