[bits 32]

extern i686_isr_handler

; CPU automatically pushes ss, esp, eflags, cs, eip

%macro ISR_NO_ERROR_CODE 1
global i686_isr%1
i686_isr%1:
    push 0              ; push dummy error code
    push %1             ; push interrupt number
    jmp isr_common
%endmacro

%macro ISR_WITH_ERROR_CODE 1
global i686_isr%1
i686_isr%1:
    ; cpu pushes an error code
    push %1             ; push interrupt number
    jmp isr_common
%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common:
    pusha               ; push in order eax, ecx, edx, ebx, esp, ebp, esi, edi
    
    ; only ds needs to be pushed because segmentation is not used
    xor eax, eax
    mov ax, ds
    push eax

    ; use kernel data segment
    mov ax, 10h
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pass esp to C function to access previously pushed information
    call i686_isr_handler
    add esp, 4

    ; restore old data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; restore general purpose registers
    add esp, 8          ; remove error code and interrupt number

    iret                ; pops cs, eip, eflags, ss, esp