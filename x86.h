#ifndef MIOS_X86_H
#define MIOS_X86_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
  volatile uint8_t data;
  asm volatile ("in %0, %1" : "=a"(data) : "d"(port));
  return data;
}

static inline void outb(uint16_t port, uint8_t data) {
  asm volatile ("out %0, %1" :: "d"(port), "a"(data));
}

static inline void sti(void) {
  asm volatile ("sti");
}

static inline void hang(void) {
  while(1) {
    asm volatile ("cli");
    asm volatile ("hlt");
  }
}

#endif // MIOS_X86_H
