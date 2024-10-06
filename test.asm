BITS 64
section .text
_start:
    call main
    xor rax, rax
    ret

main:
    push rbp
    mov rbp, rsp
    sub rsp,20
    mov al,100
    push rax
    push NULL
    mov eax,[rbp-20]
    push rax
    pop rax
    pop rbx
    pop rcx
    call test
    mov bl,0
    push rbx
    mov ebx,0
    push rbx
    push NULL
    pop rax
    pop rbx
    pop rcx
    call test
    push NULL
    pop rax
    call print
    mov ebx,[rbp-14]
    push rbx
    pop rax
    call print
