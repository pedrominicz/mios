#ifndef MIOS_X86_H
#define MIOS_X86_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t data) {
  asm volatile ("out %0, %1" :: "d"(port), "a"(data));
}

#endif // MIOS_X86_H
