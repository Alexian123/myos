#include "isr.h"

#include "idt.h"
#include "gdt.h"
#include "io.h"

#include <stddef.h>
#include <stdio.h>

#define NUM_INTERRUPTS 256

static const char* const g_exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

static isr_handler g_handlers[NUM_INTERRUPTS];

extern void i686_ISR_initGates();

void i686_ISR_init() {
    i686_ISR_initGates();
    for (int i = 0; i < NUM_INTERRUPTS; ++i) {
        i686_IDT_enableGate(i);
    }
}

void i686_ISR_registerHandler(int interrupt, isr_handler handler) {
    g_handlers[interrupt] = handler;
    i686_IDT_enableGate(interrupt);
}

void __attribute__((cdecl)) i686_isr_handler(Registers* regs) {
    if (g_handlers[regs->interrupt] != NULL) {
        g_handlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 32) {
        printf("Interrupt 0x%x not handled!\n", regs->interrupt);
    } else {
        printf("Unhandled exception 0x%x: %s!\n", regs->interrupt, g_exceptions[regs->interrupt]);
        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);
        printf("Kernel panic!\n");
        i686_panic();
    }
}