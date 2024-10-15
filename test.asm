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
	mov word [rbp-1], 0
	mov word [rbp-2], 0
	mov NULL [rbp-10], 0
	mov 
	mov dword [rbp-16], 0
	mov 
    mov ax,[rbp-16]
    mov bx,5
    mov bx,1
    add rbx,rbx
    mov cx,3
    add rbx,rcx
    mov rax,rbx
    mov ecx,[rbp-14]
    mov edx,[rbp-20]
    mov r8d,5
    mov r8d,2
    sub r8,r8
    mov r9d,8
    mov r9d,1
    mov r9d,2
    add r9,r9
    shr r9,r9
    add r8,r9
    add rdx,r8
    mov rcx,rdx
    mov r10d,[rbp-14]
    mov r11d,5
    mov r11d,1
    add r11,r11
    mov r12d,3
    mov r12b,100
    push r12
    mov r12d,5
    mov r12d,2
    add r12,r12
    push r12
    mov r12d,[rbp-20]
    push r12
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov ecx,5
    mov ecx,3
    mov ecx,4
    sub rcx,rcx
    mov r12d,1
    add rcx,r12
    add rcx,rcx
    mov rax,rcx
    mov r12b,0
    push r12
    mov r12d,0
    push r12
    mov r12d,[rbp-14]
    mov r13d,8
    add r12,r13
    push r12
    pop rax
    pop rbx
    pop rcx
    call test
    mov eax,[rbp-14]
    mov r12d,8
    add rax,r12
    push rax
    pop rax
    call print
    mov eax,[rbp-14]
    push rax
    pop rax
    call print
    mov al,[rbp-2]
    mov r12b,'h'
    mov rax,r12
    mov r12,[rbp-10]
    mov r13,524421
    mov r12,r13
    mov r13,[rbp-10]
    mov r13,113
    add r13,r13
    mov r13b,[rbp-1]
    mov r13,r14
    mov r14d,[rbp-14]
    mov r15d,5
    shl r14,r15
    mov rsp, rbp
    pop rbp
    ret

test:
    ret


section .data
main_À3	mÿ: db "he",0x22,"llo",0
