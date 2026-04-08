#include "gdt.h"

#include <stdint.h>

// helper macros
#define GDT_LIMIT_LOW(limit)                ((limit) & 0xFFFF)
#define GDT_BASE_LOW(base)                  ((base) & 0xFFFF)
#define GDT_BASE_MID(base)                  (((base) >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    ((((limit) >> 16) & 0xF) | ((flags) & 0xF0))
#define GDT_BASE_HIGH(base)                 (((base) >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MID(base),                         \
    (access),                                   \
    GDT_FLAGS_LIMIT_HI(limit, flags),           \
    GDT_BASE_HIGH(base)                         \
}

typedef struct
{
    uint16_t limitLo;               // limit (bits 0-15)
    uint16_t baseLo;                // base (bits 0-15)
    uint8_t baseMi;                 // base (bits 16-23)
    uint8_t access;                 // access flags
    uint8_t flagsLimitHi;           // limit (bits 16-19) | flags
    uint8_t baseHi;                 // base (bits 24-31)
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t limit;                 // sizeof(GDT) - 1
    GDTEntry* ptr;              // address of GDT 
} __attribute__((packed)) GDTDescriptor;

typedef enum
{
    GDT_ACCESS_CODE_READABLE            = 0x02,
    GDT_ACCESS_DATA_WRITEABLE           = 0x02,

    GDT_ACCESS_CODE_CONFORMING          = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL    = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN      = 0x04,

    GDT_ACCESS_DATA_SEGMENT             = 0x10,
    GDT_ACCESS_CODE_SEGMENT             = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS           = 0x00,

    GDT_ACCESS_RING0                    = 0x00,
    GDT_ACCESS_RING1                    = 0x20,
    GDT_ACCESS_RING2                    = 0x40,
    GDT_ACCESS_RING3                    = 0x60,

    GDT_ACCESS_PRESENT                  = 0x80
} GDTAccess;

typedef enum
{
    GDT_FLAG_64BIT                      = 0x20,
    GDT_FLAG_32BIT                      = 0x40,
    GDT_FLAG_16BIT                      = 0x00,
    GDT_FLAG_GRANULARITY_1B             = 0x00,
    GDT_FLAG_GRANULARITY_4KB            = 0x80
} GDTFlags;

static GDTEntry g_GDT[] = {
    // null descriptor
    GDT_ENTRY(0, 0, 0, 0),
    
    // Kernel 32bit code segment
    GDT_ENTRY(
        0, 
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4KB
    ),
    
    // Kernel 32bit data segment
    GDT_ENTRY(
        0, 
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4KB
    )
};

static GDTDescriptor g_GDTDescriptor = { 
    .limit = sizeof(g_GDT) - 1, 
    .ptr = g_GDT 
};

extern void __attribute__((cdecl)) i686_GDT_load(GDTDescriptor* desc, uint16_t codeSeg, uint16_t dataSeg);

void i686_GDT_init() {
    i686_GDT_load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT_OFFSET, i686_GDT_DATA_SEGMENT_OFFSET);
}