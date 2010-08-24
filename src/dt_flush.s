[global gdt_flush]
[global idt_flush]
[global tss_flush]

gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    jmp 0x08:.reload_cs
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    ret

idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

tss_flush:
    mov ax, 0x28
    ltr ax
    ret
