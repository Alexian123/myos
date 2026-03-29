#ifndef __FAT_H__
#define __FAT_H__

#include <stdint.h>
#include <stdbool.h>

#include "disk.h"

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
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct
{
    int handle;
    bool isDirectory;
    uint32_t position;
    uint32_t size;
} FAT_File;

enum FAT_Attributes
{
    FAT_ATTRIB_READ_ONLY        = 0x01,
    FAT_ATTRIB_HIDDEN           = 0x02,
    FAT_ATTRIB_SYSTEM           = 0x04,
    FAT_ATTRIB_VOLUME_ID        = 0x08,
    FAT_ATTRIB_DIRECTORY        = 0x10,
    FAT_ATTRIB_ARCHIVE          = 0x20,
    FAT_ATTRIB_LFN              = FAT_ATTRIB_READ_ONLY | FAT_ATTRIB_HIDDEN | FAT_ATTRIB_SYSTEM | FAT_ATTRIB_VOLUME_ID
};

bool FAT_init(DISK* disk);
FAT_File* FAT_open(DISK* disk, const char* path);
uint32_t FAT_read(DISK* disk, FAT_File* file, uint32_t numBytes, void *dataOut);
bool FAT_readEntry(DISK* disk, FAT_File* file, FAT_DirectoryEntry *entryOut);
void FAT_close(FAT_File* file);

#endif
