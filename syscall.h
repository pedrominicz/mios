#ifndef MIOS_SYSCALL_H
#define MIOS_SYSCALL_H

#include <stdint.h>

#define SYSCALL_CLEAR           1
#define SYSCALL_PRINT_DECIMAL   2
#define SYSCALL_PRINT_HEX       3
#define SYSCALL_PRINT_HEX8      4
#define SYSCALL_PRINT_HEX16     5
#define SYSCALL_PRINT_HEX32     6
#define SYSCALL_PRINT_HEX64     7
#define SYSCALL_PUTCHAR         8
#define SYSCALL_PRINT           9

static inline void syscall_clear(void) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_CLEAR));
}

static inline void syscall_print_decimal(uintmax_t n) {
  const uint32_t low = n;
  const uint32_t high = n >> 32;
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_DECIMAL), "b"(low), "c"(high));
}

static inline void syscall_print_hex(uintmax_t n) {
  const uint32_t low = n;
  const uint32_t high = n >> 32;
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_HEX), "b"(low), "c"(high));
}

static inline void syscall_print_hex8(uint8_t n) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_HEX8), "b"(n));
}

static inline void syscall_print_hex16(uint16_t n) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_HEX16), "b"(n));
}

static inline void syscall_print_hex32(uint32_t n) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_HEX32), "b"(n));
}

static inline void syscall_print_hex64(uint64_t n) {
  const uint32_t low = n;
  const uint32_t high = n >> 32;
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT_HEX64), "b"(low), "c"(high));
}
static inline void syscall_putchar(const char c) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_PUTCHAR), "b"(c));
}

static inline void syscall_print(const char* s) {
  asm volatile ("int $0x80" :: "a"(SYSCALL_PRINT), "b"(s));
}

#endif // MIOS_SYSCALL_H
