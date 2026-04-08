[bits 32]

;
; void __attribute__((cdecl)) i686_IDT_load(IDTDescriptor* desc);
; 
global i686_IDT_load
i686_IDT_load:
    ; new call frame
    push ebp            ; save old call frame
    mov ebp, esp        ; init new call frame

    ; load IDT
    mov eax, [ebp + 8]
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret