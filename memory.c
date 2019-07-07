#include "memory.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

typedef union Page {
  union Page* next;
  uint32_t _[1024];
} Page;

Page* free_pages;

void init_kernel_page_directory(void) {
  extern uint32_t kernel_page_directory[1024]; // Defined in "entry.S".

  for(size_t i = 0; i < 1024; ++i) {
    const uint32_t page_address = (i << 22) - KERNEL_OFFSET;
    kernel_page_directory[i] = 0;
    kernel_page_directory[i] |= page_address & 0xffc00000;
    kernel_page_directory[i] |= 0x83;
  }

  switch_kernel_page_directory();
}

static uint64_t clamp(uint64_t n, const uint64_t min, const uint64_t max) {
  n = n > min ? n : min;
  n = n > max ? max : n;
  return n;
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

  extern const uintptr_t multiboot_info_address; // Defined in "entry.S".
  const MultibootInfo* const multiboot_info =
    physical_to_virtual(multiboot_info_address);

  const MultibootMemoryRegion* memory_map =
    physical_to_virtual(multiboot_info->memory_map_address);
  const MultibootMemoryRegion* const memory_map_end =
    physical_to_virtual(multiboot_info->memory_map_address + multiboot_info->memory_map_size);

  extern const int kernel_end[]; // Defined in "kernel.lds".
  const uint64_t first_page =
    (virtual_to_physical(kernel_end) + 4095) & 0xfffff000;

  for(; memory_map < memory_map_end; ++memory_map) {
    if(memory_map->type != 1) continue;

    const uint64_t region_start =
      clamp(memory_map->address, first_page, 0x100000000);
    const uint64_t region_end =
      clamp(memory_map->address + memory_map->size, first_page, 0x100000000);

    uint64_t page = (region_start + 4095) & 0xfffff000;
    const uint64_t end = region_end - (region_end % 4096);

    for(; page < end; page += 4096) {
      Page* new_page = physical_to_virtual(page);
      new_page->next = free_pages;
      free_pages = new_page;
    }
  }
}

void free_page(void* page) {
  Page* new_page = page;
  for(size_t i = 0; i < 1024; ++i) {
    new_page->_[i] = 0;
  }
  new_page->next = free_pages;
  free_pages = new_page;
}

void* malloc_page(void) {
  Page* ret = free_pages;
  if(ret) free_pages = ret->next;
  return ret;
}

void switch_kernel_page_directory(void) {
  extern uint32_t kernel_page_directory[1024]; // Defined in "entry.S".

  asm volatile ("mov %0, %%cr3" :: "r"(virtual_to_physical(kernel_page_directory)));
}
