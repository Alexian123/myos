#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    uint8_t bootJumpInstruction[3];
    uint8_t oemID[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fatCount;
    uint16_t dirEntriesCount;
    uint16_t totalSectors;
    uint8_t mediaDescriptorType;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t largeSectorCount;
    
    // Extended boot record
    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;          // serial number, can be anything
    uint8_t volumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t systemID[8];        // 8 bytes, padded with spaces 

    // code does not matter here
} __attribute__((packed)) BootSector;

typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t createdTimeTenths;
    uint16_t createdTime;
    uint16_t createdDate;
    uint16_t accessedDate;
    uint16_t firstClusterHigh;
    uint16_t modifiedTime;
    uint16_t modifiedDate;
    uint16_t firstClusterLow;
    uint32_t size;
} __attribute__((packed)) DirectoryEntry;

BootSector g_bootSector;
uint8_t *g_fat = NULL;
DirectoryEntry *g_rootDir = NULL;
uint32_t g_rootDirEnd;

bool readBootSector(FILE *disk) {
    return fread(&g_bootSector, sizeof(g_bootSector), 1, disk) > 0;
}

bool readSectors(FILE *disk, uint32_t lba, uint32_t count, void *outBuffer) {
    bool success = true;
    success = success && (fseek(disk, lba * g_bootSector.bytesPerSector, SEEK_SET) == 0);
    success = success && (fread(outBuffer, g_bootSector.bytesPerSector, count, disk) == count);
    return success;
}

bool readFat(FILE *disk) {
    g_fat = (uint8_t*) malloc(g_bootSector.sectorsPerFat * g_bootSector.bytesPerSector);
    if (!g_fat) {
        return false;
    }
    return readSectors(disk, g_bootSector.reservedSectors, g_bootSector.sectorsPerFat, g_fat);
}

bool readRootDir(FILE *disk) {
    uint32_t lba = g_bootSector.reservedSectors + g_bootSector.sectorsPerFat * g_bootSector.fatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_bootSector.dirEntriesCount;
    uint32_t sectors = size / g_bootSector.bytesPerSector;
    if (size % g_bootSector.bytesPerSector > 0) {   // round up
        ++sectors;
    }
    g_rootDirEnd = lba + sectors;
    g_rootDir = (DirectoryEntry*) malloc(sectors * g_bootSector.bytesPerSector);
    return readSectors(disk, lba, sectors, g_rootDir);
}

DirectoryEntry* findFile(const char* name) {
    for (uint32_t i = 0; i < g_bootSector.dirEntriesCount; ++i) {
        if (memcmp(name, g_rootDir[i].name, 11) == 0) {
            return &g_rootDir[i];
        }
    }
    return NULL;
}

bool readFile(DirectoryEntry *file, FILE *disk, uint8_t *outBuffer) {
    bool ok = true;
    uint16_t currentCluster = file->firstClusterLow;

    do {
        uint32_t lba = g_rootDirEnd + (currentCluster - 2) * g_bootSector.sectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_bootSector.sectorsPerCluster, outBuffer);
        outBuffer += g_bootSector.sectorsPerCluster * g_bootSector.bytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0) {
            currentCluster = (*(uint16_t*)(g_fat + fatIndex)) & 0x0FFF;
        } else {
            currentCluster = (*(uint16_t*)(g_fat + fatIndex)) >> 4;
        }
    } while (ok && currentCluster < 0x0FF8);

    return ok;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk-image> <file-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Error opening disk image: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Error reading boot sector from disk image: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Error reading FAT from disk image: %s\n", argv[1]);
        free(g_fat);
        exit(EXIT_FAILURE);
    }

    if (!readRootDir(disk)) {
        fprintf(stderr, "Error reading root directory from disk image: %s\n", argv[1]);
        free(g_fat);
        free(g_rootDir);
        exit(EXIT_FAILURE);
    }

    DirectoryEntry *file = findFile(argv[2]);
    if (!file) {
        fprintf(stderr, "Could not find file '%s' in disk image: %s\n", argv[2], argv[1]);
        free(g_fat);
        free(g_rootDir);
        exit(EXIT_FAILURE);
    }

    uint8_t *buff = (uint8_t*) malloc(file->size + g_bootSector.bytesPerSector); // allocated 1 extra sector for safety
    if (!buff || !readFile(file, disk, buff)) {
        fprintf(stderr, "Could not read file '%s' from disk image: %s\n", argv[2], argv[1]);
        free(buff);
        free(g_fat);
        free(g_rootDir);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < file->size; ++i) {
        if (isprint(buff[i])) {
            fputc(buff[i], stdout);
        } else {
            printf("<%02x>", buff[i]);
        }
    }
    printf("\n");

    free(buff);
    free(g_rootDir);
    free(g_fat);
    fclose(disk);
    return 0;
}