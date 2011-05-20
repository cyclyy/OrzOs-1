[bits 64]
[global flushGDT]
[global flushIDT]
[global flushTSS]

flushGDT:
    mov rax, rdi
    lgdt [rax]
    mov rbx, .reload_cs
    jmp rbx
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    mov ss, ax
    
    ret

flushIDT:
    mov rax, rdi
    lidt [rax]
    ret

flushTSS:
    mov ax, 0x28
    ltr ax
    ret
