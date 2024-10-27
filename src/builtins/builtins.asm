
global _start
_start:
    mov rax, 0xb00b5
    call printLine_i64
    xor rdi, rdi
    mov rax, 60
    syscall

%include "src/builtins/utils.asm"
%include "src/builtins/print.asm"
