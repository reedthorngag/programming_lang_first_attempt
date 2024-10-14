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
    mov ax,[rbp-16]
    mov bx,5
    mov bx,1
    add rbx,rbx
    mov cx,3
    add rbx,rcx
    mov rax,rbx
    mov ecx,[rbp-14]
    mov edx,[rbp-20]
    mov R8D,5
    mov R8D,2
   unsupported OP! (mul)
    mov R9D,8
    mov R9D,1
    mov R9D,2
    add R9,R9
    shr R9,R9
    add R8,R9
    add rdx,R8
    mov rcx,rdx
    mov R1{}D,[rbp-14]
    mov R11D,5
    mov R11D,1
    add R11,R11
    mov R12D,3
   unsupported OP! (mul)
    add R1{},R11
    mov R12B,100
    push R12
    mov R12D,5
    mov R12D,2
    add R12,R12
    push R12
    mov R12D,[rbp-20]
    push R12
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov ecx,5
    mov ecx,3
    mov ecx,4
    sub rcx,rcx
    mov R12D,1
    add rcx,R12
   unsupported OP! (mul)
    mov rax,rcx
    mov R12B,0
    push R12
    mov R12D,0
    push R12
    mov R12D,[rbp-14]
    mov R13D,8
    add R12,R13
    push R12
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov R12D,8
    add rax,R12
    push rax
    pop rax
    call print
    mov eax,[rbp-14]
    push rax
    pop rax
    call print
    mov al,[rbp-2]
    mov R12B,'h'
    mov rax,R12
    mov R12,[rbp-10]
    mov R13,524421
    mov R12,R13
    mov R13,[rbp-10]
    mov R13,113
    add R13,R13
    mov R13B,[rbp-1]
    mov R13,R14
    mov R14D,[rbp-14]
    mov R15D,5
    shl R14,R15
    mov rsp, rbp
    pop rbp
    ret

test:
    ret

