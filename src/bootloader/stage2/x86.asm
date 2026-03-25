bits 16

section _TEXT class=CODE

;
; U4M
; Operation:      integer four byte multiply
; Inputs:         DX;AX   integer M1
;                 CX;BX   integer M2
; Outputs:        DX;AX   product
; Volatile:       CX, BX destroyed
;
global __U4M
__U4M:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; m1 in edx
    mov eax, edx        ; m1 in eax

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; m2 in ecx

    mul ecx             ; result in edx:eax (we only need eax)
    mov edx, eax        ; move upper half to dx
    shr edx, 16

    ret

;
; U4D
;
; Operation:      Unsigned 4 byte divide
; Inputs:         DX;AX   Dividend
;                 CX;BX   Divisor
; Outputs:        DX;AX   Quotient
;                 CX;BX   Remainder
; Volatile:       none
;
global __U4D
__U4D:
global __U4D
__U4D:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; edx - dividend
    mov eax, edx        ; eax - dividend
    xor edx, edx

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; ecx - divisor

    div ecx             ; eax - quot, edx - remainder
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

;
; bool _cdecl x86_Disk_Reset(uint8_t drive);
;
global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov dl, [bp + 4]        ; dl = drive
    mov ah, 0
    stc
    int 13h

    mov ax, 1               ; 1 = success
    sbb ax, 0               ; 0 = failure (subtract carry flag)

    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_Read(
;     uint8_t drive, 
;     uint16_t cylinder, 
;     uint16_t head, 
;     uint16_t sector, 
;     uint8_t count, 
;     void far* dataOut);
;
global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4]        ; dl = drive
    
    mov ch, [bp + 6]        ; ch = cylinder lower 8 bits
    mov cl, [bp + 7]        ; cl = cylinder bits 6-7 (six seven)
    shl cl, 6

    mov dh, [bp + 8]        ; dh = head

    mov al, [bp + 10]
    and al, 3Fh
    or cl, al               ; add sector bits 0-5 to cl

    mov al, [bp + 12]       ; al = count

    mov bx, [bp + 16]       ; es:bx = output data far ptr
    mov es, bx
    mov bx, [bp + 14]

    mov ah, 02h
    stc
    int 13h

    mov ax, 1               ; 1 = success
    sbb ax, 0               ; 0 = failure (subtract carry flag)

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret

; bool _cdecl x86_Disk_GetDriveParams(
;     uint8_t drive,
;     uint8_t* typeOut,
;     uint16_t* cylindersOut,
;     uint16_t* sectorsOut,
;     uint16_t* headsOut);
;
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push si
    push di
    push bx

    mov dl, [bp + 4]        ; dl = drive
    mov ah, 08h
    mov di, 0
    mov es, di              ; es:di = 0000:0000
    stc
    int 13h
    
    mov ax, 1               ; 1 = success
    sbb ax, 0               ; 0 = failure (subtract carry flag)

    mov si, [bp + 6]        ; typeOut ptr
    mov [si], bl

    mov bl, ch              ; cylinders lower bits in ch
    mov bh, cl              ; cylinders upper bits in cl (6-7)
    shr bh, 6
    mov si, [bp + 8]        ; cylindersOut ptr
    mov [si], bx

    xor ch, ch              ; sectors lower 5 bits in cl
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx

    mov cl, dh              ; heads in dh
    mov si, [bp + 12]
    mov [si], cx

    pop bx
    pop di
    pop si
    pop es

    mov sp, bp
    pop bp
    ret

;
; void _cdecl x86_Div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotOut, uint32_t* remOut)
;
global _x86_Div64_32
_x86_Div64_32:
    push bp
    mov bp, sp

    push bx

    ; divide upper 32 bits
    mov eax, [bp + 8]       ; eax = upper 32 bits of dividend
    mov ecx, [bp + 12]      ; ecx = divisor
    xor edx, edx
    div ecx                 ; eax = quot, edx = rem

    ; store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; divide lower 32 bits
    mov eax, [bp + 4]       ; eax = lower 32 bits of dividend
                            ; edx = old rem
    div ecx

    ; store lower 32 bits of quotient and the remainder
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    mov sp, bp
    pop bp
    ret
;
; void _cdecl x86_Video_WriteCharTTY(char c, uint8_t page)
;  - calls int 10h with ah=0Eh
;
global _x86_Video_WriteCharTTY
_x86_Video_WriteCharTTY:
    ; create new call frame
    push bp             ; save old frame
    mov bp, sp          ; init new frame

    push bx             ; save bx

    ; [bp + 0] = old call frame
    ; [bp + 2] = ret address (small memory model) = 2 bytes
    ; [bp + 4] = first arg
    ; [bp + 6] = second arg
    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]
    int 10h

    pop bx              ; restore bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret
