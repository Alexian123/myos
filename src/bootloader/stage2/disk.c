#include "disk.h"
#include "x86.h"

static void _DISK_lbaToCHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut);

bool DISK_init(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads)) {
        return false;
    }

    disk->id = driveNumber;
    disk->cylinders = cylinders + 1;
    disk->sectors = sectors;
    disk->heads = heads + 1;

    return true;
}

bool DISK_readSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut) {
    uint16_t cylinder, head, sector;
    _DISK_lbaToCHS(disk, lba, &cylinder, &head, &sector);

    // attempt to read from the disk multiple times
    for (int i = 0; i < 3; ++i) {
        if (x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut)) {
            return true;
        }
        x86_Disk_Reset(disk->id);
    }

    return false;
}

void _DISK_lbaToCHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut) {
    // sector = (LBA % sectors_per_track + 1)
    *sectorOut = lba % disk->sectors + 1;

    // cylinder = (LBA / sectors_per_track) / heads
    *cylinderOut = (lba / disk->sectors) / disk->heads;

    // head = (LBA / sectors_per_track) % heads
    *headOut = (lba / disk->sectors) % disk->heads;
}
