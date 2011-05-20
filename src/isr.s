[bits 64]
%macro  multipush 1-*
    %rep  %0
        push    %1
    %rotate 1
    %endrep
%endmacro
%macro  multipop 1-*
    %rep  %0
    %rotate -1
        pop    %1
    %endrep
%endmacro
%macro DEF_ISR_NOERRCODE 1  ; define a macro, taking one parameter
  [GLOBAL isr%1]        ; %1 accesses the first parameter.
  isr%1:
    cli
    push qword $0
    push qword %1
    jmp isr_common_stub
%endmacro

%macro DEF_ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push qword %1
    jmp isr_common_stub
%endmacro 

%macro DEF_IRQ 2      
  [GLOBAL irq%1]  
  irq%1:
    cli
    push qword $0
    push qword %2
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

[extern isrDispatcher]

isr_common_stub:
    multipush rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11
    lea rdi, [rsp + 12*8]
    call isrDispatcher
    multipop  rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11
    add rsp, 16
    sti
    iretq

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

[extern irqDispatcher]

irq_common_stub:
    multipush rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11
    lea rdi, [rsp + 12*8]
    call irqDispatcher
    multipop rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11
    add rsp, 16
    sti
    iretq

