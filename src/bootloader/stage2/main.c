#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive) {
    printf("Hello, Stage2 on boot drive %u!\r\n", bootDrive);
    
    DISK disk;
    if (!DISK_init(&disk, bootDrive)) {
        printf("ERROR: DISK: Failed to initialize disk!\r\n");
        goto halt;
    }

    if (!FAT_init(&disk)) {
        printf("ERROR: FAT: Failed to initialize FAT!\r\n");
        goto halt;
    }

    // browse files in root
    FAT_File far* file = FAT_open(&disk, "/");
    FAT_DirectoryEntry entry;
    int k = 0;
    while (FAT_readEntry(&disk, file, &entry) && k++ < 5) {
        printf("  ");
        for (int i = 0; i < 11; ++i) {
            putc(entry.name[i]);
        }
        printf("\r\n");
    }
    FAT_close(file);

    int count = 5;
    char buff[100];
    uint32_t read;
    file = FAT_open(&disk, "test.txt");
    while (read = FAT_read(&disk, file, sizeof(buff), buff)) {
        for (uint32_t i = 0; i < read; ++i) {
            if (buff[i] == '\n') {
                putc('\r');
            }
            putc(buff[i]);
        }
        if (--count <= 0) {
            break;
        }
    }
    FAT_close(file);

halt:
    for (;;);
}
