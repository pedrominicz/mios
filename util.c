#include "terminal.h"
#include "util.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline void print_string(const char* s) {
  if(!s) return;
  for(char c = *s; c != 0; c = *(++s)) {
    putchar(c);
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
    putchar(pad_zero ? '0' : ' ');
  }

  print_string(s + i);
}

void putchar(char c) {
  terminal_putchar(c);
}

void printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  for(; *format; ++format) {
    if(*format != '%') {
      putchar(*format);
      continue;
    }

    const char* start = format;
    ++format;

    // Flags field.
    bool pad_zero = false;
    switch(*format) {
    case '0':
      pad_zero = true;
      // Fallthrough.
    case ' ':
      ++format;
      break;
    }

    // Width field.
    size_t pad = 0;
    for(; *format >= '0' && *format <= '9'; ++format) {
      pad *= 10;
      pad += *format - '0';
    }

    // Length field.
    bool z = false;
    if(*format == 'z') {
      z = true;
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
      if(z) {
        print_unsigned(va_arg(ap, uint64_t), hex, pad, pad_zero);
      } else {
        print_unsigned(va_arg(ap, unsigned int), hex, pad, pad_zero);
      }
      break;
    case '%': // Percent sign.
      if(start + 1 == format) {
        putchar('%');
        break;
      }
      // Fallthrough.
    default: // Unknown format sequence.
      for(; start <= format; ++start) {
        putchar(*start);
      }
      break;
    }
  }

  va_end(ap);
}
