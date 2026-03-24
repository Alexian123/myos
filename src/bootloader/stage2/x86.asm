bits 16

section _TEXT class=CODE

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