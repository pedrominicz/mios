# mios: an experimental educational operating system

mios is an educational operating system made to explore various low-level operating system technicalities. As it stands, it demonstrated various such functions:

- Virtual memory mapping and a higher-half kernel
- Interrupt handling
- Entering and exiting user mode
- System calls
- Printing to the terminal and serial ports
- Multiboot modules

Each commit contains significant progress and a "useful resources" section on the README containing key reference material. A serious attempt was made to keep the code as clean and well documented with comments as possible, so the commit history in of itself should be instructive.

You need a cross-compiler for this to work. `./run` to build and run.

### Useful resources

https://wiki.osdev.org/Initrd#Setup_in_a_multiboot-compliant_kernel

https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format

https://wiki.osdev.org/System_V_ABI

https://github.com/daviddwlee84/OperatingSystem/blob/master/Notes/XV6/slides/XV6_thread_presentation.pdf

https://www.cs.fsu.edu/~zwang/files/cop4610/Fall2016/Schedule.pdf

https://pdos.csail.mit.edu/6.828/2014/xv6/book-rev8.pdf

https://software.intel.com/sites/default/files/managed/7c/f1/253668-sdm-vol-3a.pdf (figure 7-3)
