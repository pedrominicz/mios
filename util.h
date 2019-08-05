#ifndef MIOS_UTIL_H
#define MIOS_UTIL_H

#include <stdint.h>

#ifdef DEBUG
# define DEBUG_TEST 1
#else
# define DEBUG_TEST 0
#endif

#define putchar(c) do { if(DEBUG_TEST) _putchar(c); } while(0)
#define printf(format, ...) \
  do { if(DEBUG_TEST) _printf(format, ##__VA_ARGS__); } while(0)

#define die(...) \
  do {                                  \
    printf(__VA_ARGS__);                \
    while(1) asm volatile ("cli; hlt"); \
  } while(0)

void _putchar(char c);
void _printf(const char* format, ...) __attribute__((format(printf, 1, 2)));

static inline uintmax_t max(uintmax_t a, uintmax_t b) { return a > b ? a : b; }
static inline uintmax_t min(uintmax_t a, uintmax_t b) { return a < b ? a : b; }

static inline uint8_t inb(uint16_t port) {
  volatile uint8_t data;
  asm volatile ("in %1, %0" : "=a"(data) : "d"(port));
  return data;
}

static inline void outb(uint16_t port, uint8_t data) {
  asm volatile ("out %0, %1" :: "a"(data), "d"(port));
}

#endif // MIOS_UTIL_H
