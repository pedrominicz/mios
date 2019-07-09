#include "terminal.h"
#include "util.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const char digits[16] = "0123456789abcdef";

static inline void print_string(const char* s) {
  if(!s) return;
  for(char c = *s; c != 0; c = *(++s)) {
    putchar(c);
  }
}

static inline void print_unsigned(uintmax_t x, bool is_hex, int pad, bool pad_zero) {
  char s[21];
  for(size_t i = 0; i < 20; ++i) {
    s[i] = pad_zero ? '0' : ' ';
  }
  s[20] = 0;

  int divisor = is_hex ? 16 : 10;

  size_t i = 20;
  do {
    s[--i] = digits[x % divisor];
    x /= divisor;
  } while(x);

  print_string(s + min(i, 20 - pad));
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
    while(*format >= '0' && *format <= '9') {
      pad *= 10;
      pad += *format - '0';
      ++format;
    }

    // Length field.
    bool z = false;
    if(*format == 'z') {
      z = true;
      ++format;
    }

    // Type field and print.
    bool is_hex = false;
    switch(*format) {
    case 'p': // Pointer.
      is_hex = true;
      pad = sizeof(uintptr_t) * 2;
      pad_zero = true;
      print_unsigned(va_arg(ap, uintptr_t), is_hex, pad, pad_zero);
      break;
    case 's': // String.
      print_string(va_arg(ap, char*));
      break;
    case 'x': // Unsigned hexadecimal number.
      is_hex = true;
      // Fallthrough.
    case 'u': // Unsigned decimal number.
      if(z) {
        print_unsigned(va_arg(ap, uintmax_t), is_hex, pad, pad_zero);
      } else {
        print_unsigned(va_arg(ap, unsigned int), is_hex, pad, pad_zero);
      }
      break;
    case '%': // Percent sign.
      putchar('%');
      break;
    default: // Unknown format sequence.
      while(start <= format) {
        putchar(*start);
        ++start;
      }
      break;
    }
  }

  va_end(ap);
}
