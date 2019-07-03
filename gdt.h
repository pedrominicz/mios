#ifndef MIOS_GDT_H
#define MIOS_GDT_H

#define KERNEL_CODE_SELECTOR 0x0008
#define KERNEL_DATA_SELECTOR 0x0010

#ifndef __ASSEMBLER__

void init_gdt(void);
void switch_user_mode(void);

#endif // __ASSEMBLER__

#endif // MIOS_GDT_H
