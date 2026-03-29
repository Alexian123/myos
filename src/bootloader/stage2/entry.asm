bits 16

section .entry

extern __bss_start
extern __end

extern start
global entry

entry:
    cli

    ; save boot drive
    mov [BootDrive], dl

    ; setup stack
    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; switch to protected mode
    call enableA20          ; step 2: enable A20 line (historical reasons; may want to check if not already enabled)
    call loadGDT            ; step 3: load Global Descriptor Table

    ; step 4: set protection enable flag in cr0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; step 5: far jump into protected mode code segment 
    jmp dword 08h:.pmode    ; offset 08h = 2nd entry in GDT

.pmode:
    ; this is 32bit protected mode
    [bits 32]

    ; step 6: setup segment registers
    mov ax, 10h             ; offset 10h = 3rd entry in GDT 
    mov ds, ax
    mov ss, ax

    ; clear bss (uninitialized data)
    mov edi, __bss_start
    mov ecx, __end
    sub ecx, edi
    mov al, 0
    cld
    rep stosb

    ; expect boot drive in dl, send it as argument to cstart function
    xor edx, edx
    mov dl, [BootDrive]
    push edx
    call start
    ; should not get past this point

    cli
    hlt

enableA20:
    [bits 16]
    ; disable keyboard
    call waitForInputA20
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al

    ; read control output port
    call waitForInputA20
    mov al, KbdControllerReadCtrlOutPort
    out KbdControllerCommandPort, al

    call waitForOutputA20
    in al, KbdControllerDataPort
    push eax

    ; write control output port
    call waitForInputA20
    mov al, KbdControllerWriteCtrlOutPort
    out KbdControllerCommandPort, al

    call waitForInputA20
    pop eax
    or al, 2    ; bit 2 = A20
    out KbdControllerDataPort, al

    ; enable keyboard
    call waitForInputA20
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call waitForInputA20
    ret

waitForInputA20:
    [bits 16]
    ; wait until status bit 2 of input buffer is 0
    in al, KbdControllerCommandPort     ; read from cmd port => read status byte
    test al, 2
    jnz waitForInputA20
    ret

waitForOutputA20:
    [bits 16]
    ; wait until status bit 1 of output buffer is 1
    in al, KbdControllerCommandPort
    test al, 1
    jz waitForOutputA20
    ret

loadGDT:
    [bits 16]
    lgdt [GDTDesc]
    ret

KbdControllerDataPort           equ 060h
KbdControllerCommandPort        equ 064h
KbdControllerDisableKeyboard    equ 0ADh
KbdControllerEnableKeyboard     equ 0AEh
KbdControllerReadCtrlOutPort    equ 0D0h
KbdControllerWriteCtrlOutPort   equ 0D1h

ScreenBuffer                    equ 0B8000h

GDT:        ; null descriptor
            dq 0

            ; 32bit code segment
            dw 0FFFFh               ; limit (bits 0-15) = 0xFFFFF for full 32bit range
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23)
            db 10011010b            ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 11001111b            ; granularity (4kB pages, 32bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 32bit data segment
            dw 0FFFFh               ; limit (bits 0-15) = 0xFFFFF for full 32bit range
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23)
            db 10010010b            ; access (present, ring 0, data segment, not executable, direction 0, writable)
            db 11001111b            ; granularity (4kB pages, 32bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 16bit code segment
            dw 0FFFFh               ; limit (bits 0-15) = 0xFFFFF
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23)
            db 10011010b            ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 00001111b            ; granularity (1B pages, 16bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 16bit data segment
            dw 0FFFFh               ; limit (bits 0-15) = 0xFFFFF
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23)
            db 10010010b            ; access (present, ring 0, data segment, not executable, direction 0, writable)
            db 00001111b            ; granularity (1B pages, 16bit pmode) + limit (bits 16-19)
            db 0

GDTDesc:    dw GDTDesc - GDT - 1    ; limit = size of GDT
            dd GDT                  ; address of GDT

BootDrive:  db 0