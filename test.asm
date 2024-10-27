BITS 64
section .text
global _start
_start:
    call main
    xor rdi, rdi
    mov rax, 60
    syscall

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
    mov eax,[rbp-20]
    mov ebx,2
    mov rax,rbx
    movzx qword rcx,word [rbp-16]
    mov dx,5
    mov r8w,1
    add rdx,r8
    mov r9w,3
    add rdx,r9
    mov rcx,rdx
    mov r10d,[rbp-14]
    mov r11d,[rbp-20]
    mov r12d,5
    mov r13d,2
    sub r12,r13
    mov r13d,8
    mov r14d,1
    mov r15d,2
    add r14,r15
    xchg r14,rcx
    shr r13,cl
    xchg r14,rcx
    add r12,r13
    add r11,r12
    mov r10,r11
    mov esi,[rbp-14]
    mov edi,5
    mov eax,1
    add rdi,rax
    mov ebx,3
    sub rdi,rbx
    add rsi,rdi
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
    mov rax,1500
    push rax
    pop rax
    call print_i64
    movzx qword rax,byte [rbp-2]
    mov  rbx,104
    mov rax,rbx
    mov rax,1500
    push rax
    pop rax
    call print_i64
    mov rax,[rbp-10]
    mov rbx,524421
    mov rax,rbx
    mov rcx,[rbp-10]
    mov rdx,113
    add rcx,rdx
    movzx qword r8,byte [rbp-1]
    mov r9,main_test_tmp_30_27
    mov r8,r9
    mov rax,1300
    push rax
    pop rax
    call print_i64
    mov eax,[rbp-14]
    mov ebx,5
    xchg rbx,rcx
    shl rax,cl
    xchg rbx,rcx
    mov rax,1600
    push rax
    pop rax
    call print_i64
    add rsp,20
    mov rsp, rbp
    pop rbp
    ret

test_i64_u32_u8:
    ret

extern print_i64
extern print_i64
extern print_i64
extern print_i64
extern print_i64
extern print_i64


section .data
main_test_tmp_30_27: db "he",0x22,"llo",0
