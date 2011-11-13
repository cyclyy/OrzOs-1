[global copyUnsafe]
copyUnsafe:
    mov rcx, rdx
.copy:
    rep movsb
[section .exTable]
    dq .copy, .fixup
[section .text]
.fixup:
    mov rax, rdx
    sub rax, rcx
    ret
