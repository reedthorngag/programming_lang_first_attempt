_start:
    call main
    xor rax, rax
    ret

main:
    push rbp
    mov rbp, rsp
    sub rsp,20
    mov rsp, rbp
    pop rbp
    ret

test:
    ret

