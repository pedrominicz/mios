#include "memory.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

extern void* kernel_end; // Defined in "kernel.lds"

void init_kernel_page_directory(void) {
  extern uint32_t kernel_page_directory[1024]; // Defined in "entry.S"

  for(size_t i = 0; i < 1024; ++i) {
    const uint32_t page_address = (i << 22) - KERNEL_OFFSET;
    kernel_page_directory[i] = 0;
    kernel_page_directory[i] |= page_address & 0xffc00000;
    kernel_page_directory[i] |= 0x83;
  }

  switch_kernel_page_directory();
}

void init_kernel_malloc(void) {
  typedef struct MultibootInfo {
    uint32_t flags;
    uint32_t _[10];
    uint32_t memory_map_size;
    uint32_t memory_map_address;
  } MultibootInfo;

  typedef struct MultibootMemoryRegion {
    uint32_t _;
    uint64_t address;
    uint64_t size;
    uint32_t type;
  } MultibootMemoryRegion;

  extern const uint32_t multiboot_info_address; // Defined in "entry.S".
  const MultibootInfo* const multiboot_info =
    physical_to_virtual(multiboot_info_address);

  terminal_print("Multiboot info flags 0x");
  terminal_print_hex32(multiboot_info->flags);
  terminal_print(".\n");

  const MultibootMemoryRegion* memory_map =
    physical_to_virtual(multiboot_info->memory_map_address);
  const MultibootMemoryRegion* const memory_map_end =
    physical_to_virtual(multiboot_info->memory_map_address + multiboot_info->memory_map_size);

  while(memory_map < memory_map_end) {
    if(memory_map->address != (uint32_t)memory_map->address) {
      terminal_print("Unaddressable memory region.\n");
    } else {
      terminal_print("Memory region from 0x");
      terminal_print_hex32(memory_map->address);
      terminal_print(" to 0x");
      terminal_print_hex32(memory_map->address + memory_map->size - 1);
      terminal_print(" type ");
      terminal_print_decimal(memory_map->type);
      terminal_print(".\n");
    }
    ++memory_map;
  }
}

void switch_kernel_page_directory(void) {
  extern uint32_t kernel_page_directory[1024]; // Defined in "entry.S"

  asm volatile ("mov %0, %%cr3" :: "r"(virtual_to_physical(kernel_page_directory)));
}
