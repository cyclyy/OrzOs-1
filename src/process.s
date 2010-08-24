; copy_physical_frame(dst, src)
[global copy_physical_frame]

copy_physical_frame:
    push ebp
    mov ebp, esp
    push ebx
    pushf
    cli

    mov edx, [ebp+8]  ; source addr
    mov ebx, [ebp+12]  ; dest addr

    mov ecx, cr0
    and ecx, 0x7fffffff
    mov cr0, ecx

    mov ecx, 1024

.loop:
    mov eax, [edx]
    mov [ebx],eax
    add ebx, 4
    add edx, 4
    dec ecx
    jnz .loop

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    sti

    popf
    pop ebx
    pop ebp
    ret

[global read_eip]
read_eip:
    pop eax;
    jmp eax;
