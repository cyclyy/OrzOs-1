%macro DEF_ISR_NOERRCODE 1  ; define a macro, taking one parameter
  [GLOBAL isr%1]        ; %1 accesses the first parameter.
  isr%1:
    cli
    push dword $0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro DEF_ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push dword %1
    jmp isr_common_stub
%endmacro 

%macro DEF_IRQ 2      
  [GLOBAL irq%1]  
  irq%1:
    cli
    push dword $0
    push dword %2
    jmp irq_common_stub
%endmacro

DEF_ISR_NOERRCODE 0 ; divide zero
DEF_ISR_NOERRCODE 1 ; debug exceptions
DEF_ISR_NOERRCODE 2 
DEF_ISR_NOERRCODE 3 ; breakpoint
DEF_ISR_NOERRCODE 4 ; overflow
DEF_ISR_NOERRCODE 5 ; bounds check
DEF_ISR_NOERRCODE 6 ; invalid opcode
DEF_ISR_NOERRCODE 7 ; coprocessor not available
DEF_ISR_ERRCODE 8   ; double fault
DEF_ISR_NOERRCODE 9 
DEF_ISR_ERRCODE 10  ; invalid tss
DEF_ISR_ERRCODE 11  ;
DEF_ISR_ERRCODE 12  ; stack exception
DEF_ISR_ERRCODE 13  ; general protection exception
DEF_ISR_ERRCODE 14  ; page fault
DEF_ISR_NOERRCODE 15 
DEF_ISR_NOERRCODE 16 
DEF_ISR_NOERRCODE 17 
DEF_ISR_NOERRCODE 18 
DEF_ISR_NOERRCODE 19 
DEF_ISR_NOERRCODE 20 
DEF_ISR_NOERRCODE 21 
DEF_ISR_NOERRCODE 22 
DEF_ISR_NOERRCODE 23 
DEF_ISR_NOERRCODE 24 
DEF_ISR_NOERRCODE 25 
DEF_ISR_NOERRCODE 26 
DEF_ISR_NOERRCODE 27 
DEF_ISR_NOERRCODE 28 
DEF_ISR_NOERRCODE 29 
DEF_ISR_NOERRCODE 30 
DEF_ISR_NOERRCODE 31 
DEF_ISR_NOERRCODE 128

[extern isr_handler]

isr_common_stub:
    pushad

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popad

    add esp, 8
    sti
    iret

DEF_IRQ 0,32
DEF_IRQ 1,33
DEF_IRQ 2,34
DEF_IRQ 3,35
DEF_IRQ 4,36
DEF_IRQ 5,37
DEF_IRQ 6,38
DEF_IRQ 7,39
DEF_IRQ 8,40
DEF_IRQ 9,41
DEF_IRQ 10,42
DEF_IRQ 11,43
DEF_IRQ 12,44
DEF_IRQ 13,45
DEF_IRQ 14,46
DEF_IRQ 15,47

[extern irq_handler]

irq_common_stub:
    pushad

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popad

    add esp, 8
    sti
    iret

