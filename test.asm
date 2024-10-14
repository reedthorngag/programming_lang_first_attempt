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
    mov al,0
    push rax
    mov eax,0
    push rax
    push NULL
    pop rax
    pop rbx
    pop rcx
    call test
    push NULL
    pop rax
    call print
    mov eax,[rbp-14]
    push rax
    pop rax
    call print
    mov rsp, rbp
    pop rbp
    ret

test:
    ret

