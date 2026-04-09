#include "hal.h"

#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>

void HAL_init() {
    i686_GDT_init();
    i686_IDT_init();
    i686_ISR_init();
}