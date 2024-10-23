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
	mov byte [rbp-1], 0
	mov byte [rbp-2], 0
	mov qword [rbp-10], 0
	mov dword [rbp-14], 0
	mov word [rbp-16], 0
	mov dword [rbp-20], 0
    movzx qword  rax,word [rbp-16]
    mov bx,5
    mov cx,1
    add rbx,rcx
    mov dx,3
    add rbx,rdx
    mov rax,rbx
    mov r8d,[rbp-14]
    mov r9d,[rbp-20]
    mov r10d,5
    mov r11d,2
    sub r10,r11
    mov r12d,8
    mov r13d,1
    mov r13d,2
    add r13,r13
    xchg r13,rcx
    shr r12,cl
    xchg r13,rcx
    add r10,r12
    add r9,r10
    mov r8,r9
    mov r14d,[rbp-14]
    mov r15d,5
    mov esi,1
    add r15,rsi
    mov edi,3
    sub r15,rdi
    add r14,r15
    mov al,100
    push rax
    mov eax,5
    mov ebx,2
    add rax,rbx
    push rax
    mov eax,[rbp-20]
    push rax
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov ecx,5
    mov edx,3
    mov r8d,4
    sub rdx,r8
    mov r9d,1
    add rdx,r9
    add rcx,rdx
    mov rax,rcx
    mov r10b,0
    push r10
    mov r10d,0
    push r10
    mov r10d,[rbp-14]
    mov r11d,8
    add r10,r11
    push r10
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov r10d,8
    add rax,r10
    push rax
    pop rax
    call print
    mov eax,[rbp-14]
    push rax
    pop rax
    call print
    movzx qword  rax,byte [rbp-2]
    mov  r12,104
    mov rax,r12
    mov r13,[rbp-10]
    mov r13,524421
    mov r13,r13
    mov r14,[rbp-10]
    mov r15,113
    add r14,r15
    movzx qword  rsi,byte [rbp-1]
    mov rdi,main_test_tmp_27_27
    mov rsi,rdi
    mov ebx,[rbp-14]
    mov ecx,5
    shl rbx,cl
    mov rsp, rbp
    pop rbp
    ret

test:
    ret

print:

section .data
main_test_tmp_27_27: db "he",0x22,"llo",0
