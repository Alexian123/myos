#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>

typedef struct
{   // reverse order in which they were pushed
    uint32_t ds;                                            // pushed manually
    uint32_t edi, esi, ebp, k_esp, ebx, edx, ecx, eax;      // pushed manually with pusha
    uint32_t interrupt;                                     // pushed manually
    uint32_t error;                                         // pushed automatically in some cases, otherwise manually
    uint32_t eip, cs, eflags, esp, ss;                      // pushed automatically by CPU
} __attribute__((packed)) Registers;

typedef void (*isr_handler)(Registers* regs);

void i686_ISR_init();
void i686_ISR_registerHandler(int interrupt, isr_handler handler);

#endif