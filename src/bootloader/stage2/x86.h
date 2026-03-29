#ifndef __X86_H__
#define __X86_H__

#include <stdint.h>
#include <stdbool.h>

extern void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);
extern uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

extern bool __attribute__((cdecl)) x86_Disk_Reset(uint8_t drive);

extern bool __attribute__((cdecl)) x86_Disk_Read(
    uint8_t drive, 
    uint16_t cylinder, 
    uint16_t sector,
    uint16_t head,
    uint8_t count,
    void* dataOut);

extern bool __attribute__((cdecl)) x86_Disk_GetDriveParams(
    uint8_t drive,
    uint8_t* typeOut,
    uint16_t* cylindersOut,
    uint16_t* sectorsOut,
    uint16_t* headsOut);    // "aight, imma head out" *headsOut*

#endif