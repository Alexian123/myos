#ifndef __X86_H__
#define __X86_H__

#include "stdint.h"

extern void _cdecl x86_Div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotOut, uint32_t* remOut);
extern void _cdecl x86_Video_WriteCharTTY(char c, uint8_t page);

#endif