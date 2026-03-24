org 0h
bits 16

; CRLF
%define ENDL 0Dh, 0Ah

start:
    mov si, msg_hello
    call puts
    
.halt:
    cli
    hlt   

; Prints string to screen using bios
; Params:
;   - di:si pointer to c-string
puts:
    ; save registers on stack
    push si
    push ax
.loop1:
    lodsb       ; load byte from ds:si into ax and increment si
    or al, al   ; set zero flag if byte is 0 ('\0')
    jz .done1

    ; call bios interrupt
    mov ah, 0Eh
    mov bh, 0
    int 10h

    jmp .loop1
.done1:
    ; restore registers from stack
    pop ax
    pop si
    ret

msg_hello: db  'Hello, Kernel!', ENDL, 0