#include "memory.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

typedef struct MultibootInfo {
  uint32_t flags;
  uint32_t _0[4];
  uint32_t module_map_count;
  uint32_t module_map_address;
  uint32_t _1[4];
  uint32_t memory_map_size;
  uint32_t memory_map_address;
} MultibootInfo;

typedef struct MultibootModuleInfo {
  uint32_t start;
  uint32_t end;
  uint32_t _[2];
} MultibootModuleInfo;

typedef struct MultibootMemoryRegion {
  uint32_t _;
  uint64_t address;
  uint64_t size;
  uint32_t type;
} MultibootMemoryRegion;

typedef union Page {
  union Page* next;
  uint32_t _[1024];
} Page;

static Page* free_pages;

void init_kernel_page_directory(void) {
  for(size_t i = 0; i < 1024; ++i) {
    const uint32_t page_address = (i << 22) - KERNEL_OFFSET;
    kernel_page_directory[i] = 0;
    kernel_page_directory[i] |= page_address & 0xffc00000;
    kernel_page_directory[i] |= 0x83;
  }

  switch_page_directory(kernel_page_directory);
}

void init_kernel_malloc(void) {
  extern const uint32_t multiboot_magic; // Defined in "entry.S".
  extern const uintptr_t multiboot_info_address; // Defined in "entry.S".
  const MultibootInfo* const multiboot_info =
    physical_to_virtual(multiboot_info_address);

  if(multiboot_magic != 0x2badb002) {
    die("Incorrect Multiboot magic (0x%08lx)!\n", multiboot_magic);
  }

  if(multiboot_info->module_map_count < 1) {
    die("No boot modules found!\n");
  }

  const MultibootModuleInfo* const module_info =
    physical_to_virtual(multiboot_info->module_map_address);

  for(size_t i = module_info->start; i < module_info->end; ++i) {
    putchar(*(char*)physical_to_virtual(i));
  }

  if(!(multiboot_info->flags & 0x40)) {
    die("Boot loader did not provide physical memory map!\n");
  }

  const MultibootMemoryRegion* memory_map =
    physical_to_virtual(multiboot_info->memory_map_address);
  const MultibootMemoryRegion* const memory_map_end =
    physical_to_virtual(multiboot_info->memory_map_address + multiboot_info->memory_map_size);

  for(; memory_map < memory_map_end; ++memory_map) {
    if(memory_map->type != 1) continue;

    const uint64_t region_start =
      min(max(memory_map->address, 0x400000), 0x8000000);
    const uint64_t region_end =
      min(max(memory_map->address + memory_map->size, 0x400000), 0x8000000);

    uint64_t page = (region_start + 4095) & 0xfffff000;
    const uint64_t end = region_end - (region_end % 4096);

    printf("Allocating memory region from 0x%08llx to 0x%08llx... ",
        memory_map->address, memory_map->address + memory_map->size);

    size_t count = 0;
    for(; page < end; page += 4096) {
      free_page(physical_to_virtual(page));
      ++count;
    }

    printf("allocated %zu pages.\n", count);
  }
}

void free_page(void* page) {
  Page* new_page = page;
  new_page->next = free_pages;
  free_pages = new_page;
}

void* malloc_page(void) {
  Page* ret = free_pages;
  if(ret) {
    free_pages = ret->next;
    for(size_t i = 0; i < 1024; ++i) {
      ret->_[i] = 0;
    }
  }
  return ret;
}
