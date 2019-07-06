#include "memory.h"
#include "terminal.h"

#include <stdint.h>

typedef struct MultibootInfo {
  uint32_t flags;
  uint32_t _[10];
  uint32_t memory_map_length;
  uint32_t memory_map_address;
} MultibootInfo;

typedef struct MultibootMemoryRegion {
  uint32_t _;
  uint64_t address;
  uint64_t size;
  uint32_t type;
} MultibootMemoryRegion;

typedef struct MemoryRegion {
  uint32_t address;
  uint32_t size;
} MemoryRegion;

extern const MultibootInfo* const multiboot_info; // Defined in "entry.S".
extern void* kernel_end; // Defined in "kernel.lds"

void init_memory(void) {
  MemoryRegion memory_region[MAX_MEMORY_REGIONS] = {0};
  (void)memory_region;

  const MultibootMemoryRegion* memory_map =
    (MultibootMemoryRegion*)multiboot_info->memory_map_address;
  const MultibootMemoryRegion* const memory_map_end =
    (MultibootMemoryRegion*)(multiboot_info->memory_map_address + multiboot_info->memory_map_length);

  while(memory_map < memory_map_end) {
    if(memory_map->address != (uint32_t)memory_map->address) {
      terminal_print("Unaddressable memory region.\n");
    } else {
      terminal_print("Memory region from 0x");
      terminal_print_hex32(memory_map->address);
      terminal_print(" size 0x");
      terminal_print_hex32(memory_map->size);
      terminal_print(" type ");
      terminal_print_decimal(memory_map->type);
      terminal_print(".\n");
    }
    ++memory_map;
  }
}
