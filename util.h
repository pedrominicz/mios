#ifndef MIOS_UTIL_H
#define MIOS_UTIL_H

#include <stdint.h>

void putchar(char c);
void printf(const char* format, ...);

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