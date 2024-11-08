BITS 64
section .text
global _start
_start:
    call main
    xor rdi, rdi
    mov rax, 60
    syscall

main:
    mov rax,20
    push rax
    mov rax,1
    push rax
    mov rax,0
    push rax
    pop rax
    pop rbx
    pop rcx
    call fibonacci_i64_i64_i64
    ret

fibonacci_i64_i64_i64:
    push rbp
    mov rbp, rsp
    sub rsp,24
	mov qword [rbp-8], 0
	mov qword [rbp-16], 0
	mov qword [rbp-24], 0
    mov [rbp-24],rax
    mov [rbp-16],rbx
    mov [rbp-8],rcx
    add rax,rbx
    push rax
    pop rax
    call printNum_i64
    mov rax,[rbp-8]
    mov rbx,0
    cmp rax,rbx
    mov rax,0
    setnz al
    cmp rax,0
    je .label_0
    mov rax,[rbp-8]
    mov rbx,1
    sub rax,rbx
    push rax
    mov rax,[rbp-24]
    mov rcx,[rbp-16]
    add rax,rcx
    push rax
    push rcx
    pop rax
    pop rbx
    pop rcx
    call fibonacci_i64_i64_i64
.label_0:
    add rsp,24
    leave
    ret

extern printNum_i64


section .data
