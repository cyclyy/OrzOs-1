[bits 64]
[global gdt_flush]
[global flushIDT]
[global tss_flush]

gdt_flush:
    mov rax, [rsp+8]
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
    
    ret

flushIDT:
    mov rax, rdi
    lidt [rax]
    ret

tss_flush:
    mov ax, 0x28
    ltr ax
    ret
