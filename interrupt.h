#ifndef MIOS_INTERRUPT_H
#define MIOS_INTERRUPT_H

#include <stdint.h>

typedef struct InterruptFrame {
  // `pusha`
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  // `push gs`
  uint16_t gs;
  uint16_t gs_padding;
  // `push fs`
  uint16_t fs;
  uint16_t fs_padding;
  // `push es`
  uint16_t es;
  uint16_t es_padding;
  // `push ds`
  uint16_t ds;
  uint16_t ds_padding;
  // `push dword %1`
  uint32_t interrupt_number;
  // `push dword 0` or pushed by the CPU.
  uint32_t interrupt_error_code;
  // Pushed by the CPU.
  uint32_t eip;
  uint16_t cs;
  uint16_t cs_padding;
  uint16_t eflags;
} InterruptFrame;

typedef void (*InterruptHandler)(const InterruptFrame* const frame);

void init_interrupt(void);
void register_interrupt_handler(uint8_t interrupt_number, InterruptHandler function);

#endif // MIOS_INTERRUPT_H
