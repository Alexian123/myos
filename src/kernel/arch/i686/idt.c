#include "idt.h"

#include <util/binary.h>

#define NUM_IDT_ENTRIES 256

typedef struct
{
    uint16_t baseLow;
    uint16_t segmentSelector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t baseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t limit;             // sizeof(IDT) - 1
    IDTEntry* ptr;              // address of IDT 
} __attribute__((packed)) IDTDescriptor;

static IDTEntry g_IDT[NUM_IDT_ENTRIES];

static IDTDescriptor g_IDTDescriptor = {
    .limit = sizeof(g_IDT) - 1,
    .ptr = g_IDT
};

extern void __attribute__((cdecl)) i686_IDT_load(IDTDescriptor* desc);

void i686_IDT_init() {
    i686_IDT_load(&g_IDTDescriptor);
}

void i686_IDT_setGate(int interrupt, void* base, uint16_t segmentDesc, uint8_t flags) {
    g_IDT[interrupt].baseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].segmentSelector = segmentDesc;
    g_IDT[interrupt].reserved = 0;
    g_IDT[interrupt].flags = flags;
    g_IDT[interrupt].baseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

void i686_IDT_enableGate(int interrupt) {
    BIT_SET(g_IDT[interrupt].flags, IDT_FLAG_PRESENT);
}

void i686_IDT_disableGate(int interrupt) {
    BIT_CLEAR(g_IDT[interrupt].flags, IDT_FLAG_PRESENT);
}