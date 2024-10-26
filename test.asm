BITS 64
section .text
_start:
    call main_null
    xor rax, rax
    ret

main_i32:
    push rbp
    mov rbp, rsp
    sub rsp,20
	mov byte [rbp-1], 0
	mov byte [rbp-2], 0
	mov qword [rbp-10], 0
	mov dword [rbp-14], 0
	mov word [rbp-16], 0
	mov dword [rbp-20], 0
    movzx qword rax,word [rbp-16]
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
    call test_i64_u32_u8
    mov eax,[rbp-14]
    mov ebx,5
    mov ecx,3
    mov edx,4
    sub rcx,rdx
    mov r8d,1
    add rcx,r8
    add rbx,rcx
    mov rax,rbx
    mov al,0
    push rax
    mov eax,0
    push rax
    mov eax,[rbp-14]
    mov ebx,8
    add rax,rbx
    push rax
    pop rax
    pop rbx
    pop rcx
    call test_i64_u32_u8
    mov eax,[rbp-14]
    mov ebx,8
    add rax,rbx
    push rax
    pop rax
    call print_i64
    mov eax,[rbp-14]
    push rax
    pop rax
    call print_i64
    movzx qword rax,byte [rbp-2]
    mov  rbx,104
    mov rax,rbx
    mov rcx,[rbp-10]
    mov rdx,524421
    mov rcx,rdx
    mov r8,[rbp-10]
    mov r9,113
    add r8,r9
    movzx qword r10,byte [rbp-1]
    mov r11,main_test_tmp_27_27
    mov r10,r11
    mov r12d,[rbp-14]
    mov r13d,5
    xchg r13,rcx
    shl r12,cl
    xchg r13,rcx
    add rsp,20
    mov rsp, rbp
    pop rbp
    ret

test_i64_u32_u8:
    ret

