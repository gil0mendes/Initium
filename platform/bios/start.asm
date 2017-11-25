global loader_entry

# Entry point
section .text
bits 32

loader_entry:
    mov si, [test_msg]
    call puts_2
    hlt
    jmp $

puts_2:
    push ax
    cld
.1:  
    lodsb
    test al, al
    jz .2
    push bx
    mov ah, 0x0E
    mov bx, 1
    int 0x10
    pop bx
    jmp .1
.2:
    pop ax
    ret

test_msg:           db "In :)", 0xA, 0xD, 0