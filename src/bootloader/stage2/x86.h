#ifndef __X86_H__
#define __X86_H__

#include "stdint.h"

extern void _cdecl x86_Div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotOut, uint32_t* remOut);

extern void _cdecl x86_Video_WriteCharTTY(char c, uint8_t page);

extern bool _cdecl x86_Disk_Reset(uint8_t drive);

extern bool _cdecl x86_Disk_Read(
    uint8_t drive, 
    uint16_t cylinder, 
    uint16_t head, 
    uint16_t sector, 
    uint8_t count, 
    void far* dataOut);

extern bool _cdecl x86_Disk_GetDriveParams(
    uint8_t drive,
    uint8_t* typeOut,
    uint16_t* cylindersOut,
    uint16_t* sectorsOut,
    uint16_t* headsOut);    // "aight, imma head out" *headsOut*

#endif
