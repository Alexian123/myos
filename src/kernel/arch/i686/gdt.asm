[bits 32]

;
; void __attribute__((cdecl)) i686_GDT_load(GDTDescriptor* desc, uint16_t codeSeg, uint16_t dataSeg)
;
global i686_GDT_load
i686_GDT_load:
    ; new call frame
    push ebp            ; save old call frame
    mov ebp, esp        ; init new call frame

    ; load GDT descriptor
    mov eax, [ebp + 8]
    lgdt [eax]
    
    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf    ; performs a far jump

.reload_cs:

    ; reload data segments
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret