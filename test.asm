_start:
    call main
    xor rax, rax
    ret

print:
    ret

main:
    push rbp
    mov rbp, rsp
    sub rsp,20
    push rax
    push rax
    push rax
    pop rax
    pop rbx
    pop rcx
    call test
    push rax
    push rax
    push rax
    pop rax
    pop rbx
    pop rcx
    call test
    push rax
    pop rax
    call print
    push rax
    pop rax
    call print
    mov rsp, rbp
    pop rbp
    ret

test:
    ret

