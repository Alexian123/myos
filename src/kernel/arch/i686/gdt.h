#ifndef __GDD_H__
#define __GDT_H__

#define i686_GDT_CODE_SEGMENT_OFFSET    0x08
#define i686_GDT_DATA_SEGMENT_OFFSET    0x10

void i686_GDT_init();

#endif