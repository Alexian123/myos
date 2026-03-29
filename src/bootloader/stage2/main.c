#include <stdint.h>

#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

typedef void (*KernelStart)();

void __attribute__((cdecl)) start(uint16_t bootDrive) {
    clrscr();

    DISK disk;
    if (!DISK_init(&disk, bootDrive)) {
        printf("ERROR: STAGE2: Failed to initialize disk!\r\n");
        goto halt;
    }

    if (!FAT_init(&disk)) {
        printf("ERROR: STAGE2: Failed to initialize FAT!\r\n");
        goto halt;
    }

    // load kernel
    FAT_File* kernelFile = FAT_open(&disk, "/kernel.bin");
    uint32_t read;
    uint8_t* kernelBuffer = Kernel;
    while ((read = FAT_read(&disk, kernelFile, MEMORY_LOAD_SIZE, KernelLoadBuffer))) {
        memcpy(kernelBuffer, KernelLoadBuffer, read);
        kernelBuffer += read;
    }
    FAT_close(kernelFile);

    // execute kernel
    KernelStart kernelStart = (KernelStart)Kernel;
    kernelStart();

halt:
    for (;;);
}
