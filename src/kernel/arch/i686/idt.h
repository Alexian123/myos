#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

typedef enum
{
    IDT_FLAG_GATE_TASK          = 0x5,
    IDT_FLAG_GATE_16BIT_INT     = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP    = 0x7,
    IDT_FLAG_GATE_32BIT_INT     = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP    = 0xF,

    IDT_FLAG_RING0              = (0 << 5),
    IDT_FLAG_RING1              = (1 << 5),
    IDT_FLAG_RING2              = (2 << 5),
    IDT_FLAG_RING3              = (3 << 5),

    IDT_FLAG_PRESENT            = 0x80
} IDTFlags;

void i686_IDT_init();

void i686_IDT_setGate(int interrupt, void* base, uint16_t segmentDesc, uint8_t flags);

void i686_IDT_enableGate(int interrupt);
void i686_IDT_disableGate(int interrupt);

#endif