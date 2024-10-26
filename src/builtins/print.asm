global printStr_str
global printLine_str
global printNum_u32
global printNum_i32
global printNum_u64
global printNum_i64
global print_char
global strLen_str


section .text

; string in rax, null terminated
printStr_str:

    mov rsi, rax
    call strLen ; preserves rsi

    ; rsi containes string pointer
    mov rdx, rax ; length
    mov rax, 1 ; exit
    mov rdi, 1 ; stdout
    syscall

    ret

; string in rax, null terminated
printLine_str:

    mov rsi, rax
    call strLen

    push rsi
    add rsi, rax
    mov byte [rsi], 0x0a ; \n
    pop rbx
    push rsi
    mov rsi, rbx

    ; rsi containes string pointer
    mov rdx, rax ; length
    mov rax, 1 ; write
    mov rdi, 1 ; stdout
    syscall

    pop rsi
    mov byte [rsi], 0

    ret

; these are readonly
hexChars db "0123456789abcdef",0
hexPrefix db "0x",0

; number to print in rax, doesnt print leading zeros
printNum_u32:
printNum_i32:
printNum_u64:
printNum_i64:
    mov byte [endChar], 0x20
    jmp printHex

printLine_u32:
printLine_i32:
printLine_u64:
printLine_i64:
    mov byte [endChar], 0x0a
    jmp printHex

printHex:

    push rax

    mov rsi, hexPrefix
    mov rdx, 2 ; length
    mov rax, 1 ; write
    mov rdi, 1 ; stdout
    syscall

    pop r8

    mov r9, 1
    mov r10, 0
    mov cl, 60
    mov rbx, 0xf000000000000000
    mov rsi, hexChars

.loop:
    mov rax, r8
    and rax, rbx
    cmovz r10, r9
    cmp r10, 1
    je .updateValues
    mov r9, 0

    shr rax, cl

    mov al, [rsi+rax]
    push rsi
    push r10
    push rcx
    push r9
    push r8
    call print_char
    pop r8
    pop r9
    pop rcx
    pop r10
    pop rsi

.updateValues:
    mov r10, 0
    sub cl, 4
    shr rbx, 4
    jnz .loop

    cmp r9, 0
    je .skipZero

    mov al, '0'
    call print_char

.skipZero:
    mov al, 0x0a
    call print_char

    ret

; prints char in al
print_char:

    mov [char], al
    mov rsi, char
    mov rdx, 1 ; length
    mov rax, 1 ; write
    mov rdi, 1 ; stdout
    syscall

    ret


strLen_str:
    mov rsi, rax

; string pointer in rsi
; returns length in rax
strLen:
    push rsi

    xor rax, rax
.loop:
    cmp byte [rsi], 0
    jz .return
    inc rax
    inc rsi
    jmp .loop

.return:
    pop rsi
    ret


section .data

char db 0

endChar db 0x20, 0 ; space, \n is 0x0a

