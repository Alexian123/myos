#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include "minmax.h"

#include <stddef.h>

#define SECTOR_SIZE         512
#define MAX_PATH_LEN        256
#define MAX_FILE_HANDLES    10 
#define ROOT_DIR_HANDLE     -1

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
} __attribute__((packed)) FAT_BootSector;

typedef struct
{
    uint8_t buffer[SECTOR_SIZE];
    FAT_File public;
    bool isOpen;
    uint32_t firstCluster;
    uint32_t currentCluster;
    uint32_t currentSectorInCluster;
} FAT_FileData;

typedef struct 
{
    union
    {
        FAT_BootSector bootSector;
        uint8_t bootSectorBytes[SECTOR_SIZE];
    } BS;
    FAT_FileData rootDirectory;
    FAT_FileData openedFiles[MAX_FILE_HANDLES];
} FAT_Data;

static FAT_Data* g_data = NULL;
static uint8_t* g_fat = NULL;   // consider replacing with a cache
static uint32_t g_dataSectionLBA;

static bool _FAT_readBootSector(DISK* disk);
static bool _FAT_readFat(DISK* disk);
static FAT_File* _FAT_openEntry(DISK* disk, FAT_DirectoryEntry* entry);
static uint32_t _FAT_clusterToLba(uint32_t cluster);
static bool _FAT_findFile(DISK* disk, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut);
static uint32_t _FAT_nextCluster(uint32_t currentCluster);

bool FAT_init(DISK* disk) {
    g_data = (FAT_Data*)MEMORY_FAT_ADDR;

    // read boot sector
    if (!_FAT_readBootSector(disk)) {
        printf("ERROR: FAT: Failed to read boot sector!\r\n");
        return false;
    }

    // read FAT
    g_fat = (uint8_t*)g_data + sizeof(FAT_Data);
    uint32_t fatSize = g_data->BS.bootSector.bytesPerSector * g_data->BS.bootSector.sectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("ERROR: FAT: Not enough memory to read FAT! Required: %lu, available: %u\r\n", 
            sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!_FAT_readFat(disk)) {
        printf("ERROR: FAT: Error reading FAT!\r\n");
        return false;
    }

    // open root dir
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_data->BS.bootSector.dirEntriesCount;
    uint32_t rootDirLBA = g_data->BS.bootSector.reservedSectors + g_data->BS.bootSector.sectorsPerFat * g_data->BS.bootSector.fatCount;

    g_data->rootDirectory.isOpen = true;
    g_data->rootDirectory.public.handle = ROOT_DIR_HANDLE;
    g_data->rootDirectory.public.isDirectory = true;
    g_data->rootDirectory.public.position = 0;
    g_data->rootDirectory.public.size = sizeof(FAT_DirectoryEntry) * g_data->BS.bootSector.dirEntriesCount;
    g_data->rootDirectory.firstCluster = rootDirLBA;
    g_data->rootDirectory.currentCluster = rootDirLBA;
    g_data->rootDirectory.currentSectorInCluster = 0;

    if (!DISK_readSectors(disk, rootDirLBA, 1, g_data->rootDirectory.buffer)) {
        printf("ERROR: FAT: Error reading root directory!\r\n");
        return false;
    }

    // calculate data section lba
    uint32_t rootDirSectors = (rootDirSize + g_data->BS.bootSector.bytesPerSector - 1) / g_data->BS.bootSector.bytesPerSector;
    g_dataSectionLBA = rootDirLBA + rootDirSectors;

    // reset opened files
    for (int i = 0; i < MAX_FILE_HANDLES; ++i) {
        g_data->openedFiles[i].isOpen = false;
    }

    return true;
}

FAT_File* FAT_open(DISK* disk, const char* path) {
    char name[MAX_PATH_LEN];
    
    if (disk == NULL || path == NULL) {
        return NULL;
    }

    // ignore leading '/'
    if (*path == '/') {
        ++path;
    }

    FAT_File* current = &g_data->rootDirectory.public;

    while (*path) {
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL) {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        } else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        // find dir entry in current dir
        FAT_DirectoryEntry entry;
        if (_FAT_findFile(disk, current, name, &entry)) {
            FAT_close(current);

            // check if it is a directory
            if (!isLast && entry.attributes & FAT_ATTRIB_DIRECTORY == 0) {
                printf("ERROR: FAT: %s is not a directory!\r\n", name);
                return NULL;
            }
            
            // open new directory entry
            current = _FAT_openEntry(disk, &entry);
        } else {
            FAT_close(current);
            printf("ERROR: FAT: %s not found!\r\n", name);
            return NULL;
        }
    }

    return current;
}

uint32_t FAT_read(DISK* disk, FAT_File* file, uint32_t numBytes, void *dataOut) {
    if (file == NULL || disk == NULL) {
        return 0;
    }
    
    // get file data
    FAT_FileData* fileData = (file->handle == ROOT_DIR_HANDLE) 
        ? &g_data->rootDirectory 
        : &g_data->openedFiles[file->handle];

    uint8_t* u8DataOut = (uint8_t*)dataOut;

    // read until EOF
    if (!fileData->public.isDirectory || (fileData->public.isDirectory && fileData->public.size != 0)) {
        numBytes = min(numBytes, fileData->public.size - fileData->public.position);
    }

    while (numBytes > 0) {
        uint32_t leftInBuffer = SECTOR_SIZE - (fileData->public.position % SECTOR_SIZE);
        uint32_t take = min(numBytes, leftInBuffer);

        memcpy(u8DataOut, fileData->buffer + fileData->public.position % SECTOR_SIZE, take);
        u8DataOut += take;
        fileData->public.position += take;
        numBytes -= take;

        // Check if more data needs to be read
        if (leftInBuffer == take) {
            // handle root dir
            if (fileData->public.handle == ROOT_DIR_HANDLE) {
                ++fileData->currentCluster;

                if (!DISK_readSectors(disk, fileData->currentCluster, 1, fileData->buffer)) {
                    printf("ERROR: FAT: Error reading inside root directory!\r\n");
                    break;  
                }
            } else {
                // calculate next sector and cluster to read
                if (++fileData->currentSectorInCluster >= g_data->BS.bootSector.sectorsPerCluster) {
                    fileData->currentSectorInCluster = 0;
                    fileData->currentCluster = _FAT_nextCluster(fileData->currentCluster);
                }

                if (fileData->currentCluster >= 0xFF8) {
                    // mark EOF
                    fileData->public.size = fileData->public.position;
                    break;
                }

                // read next sector
                if (!DISK_readSectors(
                    disk, 
                    _FAT_clusterToLba(fileData->currentCluster) + fileData->currentSectorInCluster,
                    1,
                    fileData->buffer
                )) {
                    printf("ERROR: FAT: Error reading sector!\r\n");
                    break;  
                }
            }
        }
    }

    return u8DataOut - (uint8_t*)dataOut;
}

bool FAT_readEntry(DISK* disk, FAT_File* file, FAT_DirectoryEntry *entryOut) {
    return FAT_read(disk, file, sizeof(FAT_DirectoryEntry), entryOut) == sizeof(FAT_DirectoryEntry);
}

void FAT_close(FAT_File* file) {
    if (file->handle == ROOT_DIR_HANDLE) {
        file->position = 0;
        g_data->rootDirectory.currentCluster = g_data->rootDirectory.firstCluster;
    } else {
        g_data->openedFiles[file->handle].isOpen = false;
    }
}


bool _FAT_readBootSector(DISK* disk) {
    return DISK_readSectors(disk, 0, 1, g_data->BS.bootSectorBytes);
}

bool _FAT_readFat(DISK* disk) {
    return DISK_readSectors(disk, g_data->BS.bootSector.reservedSectors, 
        g_data->BS.bootSector.sectorsPerFat, g_fat);
}

FAT_File* _FAT_openEntry(DISK* disk, FAT_DirectoryEntry* entry) {
    // find empty handle
    int handle = -2;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; ++i) {
        if (!g_data->openedFiles[i].isOpen) {
            handle = i;
        }
    }

    // out of handles
    if (handle < 0) {
        printf("ERROR: FAT: No more available file handles!\r\n");
        return false;
    }

    // setup fields
    FAT_FileData* fileData = &g_data->openedFiles[handle];
    fileData->public.handle = handle;
    fileData->public.isDirectory = (entry->attributes & FAT_ATTRIB_DIRECTORY) != 0;
    fileData->public.position = 0;
    fileData->public.size = entry->size;
    fileData->firstCluster = entry->firstClusterLow + ((uint32_t)entry->firstClusterHigh << 16);
    fileData->currentCluster = fileData->firstCluster;
    fileData->currentSectorInCluster = 0;

    if (!DISK_readSectors(disk, _FAT_clusterToLba(fileData->currentCluster), 1, fileData->buffer)) {
        printf("ERROR: FAT: Error reading directory!\r\n");
        return false;
    }

    fileData->isOpen = true;
    return &fileData->public;
}

uint32_t _FAT_clusterToLba(uint32_t cluster) {
    return g_dataSectionLBA + (cluster - 2) * g_data->BS.bootSector.sectorsPerCluster;

}

bool _FAT_findFile(DISK* disk, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut) {
    char fatName[12];
    FAT_DirectoryEntry entry;

    // convert name to fat name format
    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';

    const char* extension = strrchr(name, '.');
    if (extension == NULL) {
        extension = name + 11;
    }

    for (int i = 0; i < 8 && name[i] && name + i < extension; ++i) {
        fatName[i] = toupper(name[i]);
    }

    if (extension != name + 11) {
        for (int i = 0; i < 3 && extension[i + 1]; ++i) {
            fatName[i + 8] = toupper(extension[i + 1]);
        }
    }

    while (FAT_readEntry(disk, file, &entry)) {
        if (memcmp(fatName, entry.name, 11) == 0) {
            *entryOut = entry;
            return true;
        }
    }

    return false;
}

uint32_t _FAT_nextCluster(uint32_t currentCluster) {
    uint32_t fatIndex = currentCluster * 3 / 2;
    if (currentCluster % 2 == 0) {
        return (*(uint16_t*)(g_fat + fatIndex)) & 0x0FFF;
    } else {
        return (*(uint16_t*)(g_fat + fatIndex)) >> 4;
    }
}
