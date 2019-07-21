#include "memory.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

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

typedef union Page {
  union Page* next;
  uint32_t _[1024];
} Page;

static Page* free_pages;

// Global descriptor table (GDT).
static uint64_t gdt[6] = {
  0x0000000000000000, // Null segment.
  0x00cf9a000000ffff, // Kernel code segment.
  0x00cf92000000ffff, // Kernel data segment.
  0x00cffa000000ffff, // User mode code segment.
  0x00cff2000000ffff, // User mode data segment.
  0x0000e90000000068, // Task state segment (TSS).
};

// Task state segment (TSS).
static uint32_t tss[26] = {
  [2] = KERNEL_DATA_SEGMENT, // Kernel stack segment.
  // Make segment registers accessible from privilege level-3, i.e. user mode.
  [18] = KERNEL_DATA_SEGMENT + 3, // Extra segment.
  [19] = KERNEL_CODE_SEGMENT + 3, // Code segment.
  [20] = KERNEL_DATA_SEGMENT + 3, // Stack segment.
  [21] = KERNEL_DATA_SEGMENT + 3, // Data segment.
  [22] = KERNEL_DATA_SEGMENT + 3, // Extra segment #2 (`fs`).
  [23] = KERNEL_DATA_SEGMENT + 3, // Extra segment #3 (`gs`).
  // Setting I/O map base address beyond the TSS limit and setting I/O
  // privilege level to 0 in `eflags` disables I/O instructions in user mode.
  [25] = 0xffff0000, // I/O map base address.
};

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
  extern const uintptr_t multiboot_info_address; // Defined in "entry.S".
  const MultibootInfo* const multiboot_info =
    physical_to_virtual(multiboot_info_address);

  if(!(multiboot_info->flags & 0x00000040)) {
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

// Initialize global descriptor table (GDT).
void init_gdt(void) {
  // Task state segment base address (bits 0-23).
  gdt[5] |= ((uint64_t)(uintptr_t)tss & 0x00ffffff) << 16;
  // Task state segment base address (bits 24-31).
  gdt[5] |= ((uint64_t)(uintptr_t)tss & 0xff000000) << 32;

  volatile uint64_t gdt_descriptor = 0;
  gdt_descriptor |= sizeof(gdt) - 1;
  gdt_descriptor |= (uint64_t)(uintptr_t)gdt << 16;

  asm volatile ("lgdt (%0)" :: "r"(&gdt_descriptor));
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
