#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

extern void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);
extern uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

extern void __attribute__((cdecl)) i686_panic();

#endif