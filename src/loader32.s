;
; boot.s -- Kernel start location. Also defines multiboot header.
; Based on Bran's kernel development tutorial file start.asm
;

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
MBOOT_AOUT_KLUDGE   equ 1<<16
MBOOT_HEADER_FLAGS  equ (MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_AOUT_KLUDGE)
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
LOADBASE            equ 0x100000
VIRTUALBASE         equ 0xffffffffc0000000


[BITS 32]                       ; All instructions should be 32-bit.
[ORG 0x180000]

mboot:
dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
; 4-byte boundary in your kernel file
dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct

dd  mboot                     ; Location of this descriptor
dd  mboot                   ; Start of kernel '.text' (code) section.
dd  00                       ; End of kernel '.data' section.
dd  00                       ; End of kernel.
dd  start                     ; Kernel entry point (initial EIP).

[GLOBAL start]                ; Kernel entry point.
start:
push    0
mov     eax,esp
add     eax,4
push    eax                   ; Initial esp  
push    0
push    ebx                   ; Load multiboot header location

mov edi, 0x1000    ; Set the destination index to 0x1000.
mov cr3, edi       ; Set control register 3 to the destination index.
xor eax, eax       ; Nullify the A-register.
mov ecx, 1024*7    ; Set the C-register to 4096.
rep stosd          ; Clear the memory.
mov edi, cr3       ; Set the destination index to control register 3.

; 1)PML4E
mov DWORD [edi], 0x2003      ; Set the double word at the destination index to 0x2003.
mov DWORD [edi+0xff8], 0x3003      ; Set the double word at the destination index to 0x2003.
add edi, 0x1000

; 2)PDPE-first       
mov DWORD [edi], 0x4003      ; Set the double word at the destination index to 0x3003.
add edi, 0x1000

; 3)PDPE-last
mov DWORD [edi+0xff8], 0x5003      ; Set the double word at the destination index to 0x2003.
add edi, 0x1000

; 4)PDE-first
mov DWORD [edi], 0x6003      ; Set the double word at the destination index to 0x4003.
add edi, 0x1000              ; Add 0x1000 to the destination index.

; 5)PDE-last
mov DWORD [edi], 0x7003      ; Set the double word at the destination index to 0x4003.
add edi, 0x1000

; 6)PTE-first
mov ebx, 0x00000003          ; Set the B-register to 0x00000003.
mov ecx, 512                 ; Set the C-register to 512.

.SetEntry1:
mov DWORD [edi], ebx         ; Set the double word at the destination index to the B-register.
add ebx, 0x1000              ; Add 0x1000 to the B-register.
add edi, 8                   ; Add eight to the destination index.
loop .SetEntry1               ; Set the next entry.

; 7)PTE-last
mov ebx, 0x00000003          ; Set the B-register to 0x00000003.
mov ecx, 512                 ; Set the C-register to 512.

.SetEntry2:
mov DWORD [edi], ebx         ; Set the double word at the destination index to the B-register.
add ebx, 0x1000              ; Add 0x1000 to the B-register.
add edi, 8                   ; Add eight to the destination index.
loop .SetEntry2               ; Set the next entry.

mov eax, cr4                 ; Set the A-register to control register 4.
or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
mov cr4, eax                 ; Set control register 4 to the A-register.

mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
rdmsr                        ; Read from the model-specific register.
or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
wrmsr                        ; Write to the model-specific register.

mov eax, cr0                 ; Set the A-register to control register 0.
or eax, 1 << 31              ; Set the PG-bit, which is the 32nd bit (bit 31).
mov cr0, eax                 ; Set control register 0 to the A-register.

lgdt [gdt64.pointer]         ; Load the 64-bit global descriptor table.
jmp gdt64.code:start64       ; Set the code segment and enter 64-bit long mode.

; Use 64-bit.
[BITS 64]

start64:
cli                           ; Clear the interrupt flag.
mov ax, gdt64.data            ; Set the A-register to the data descriptor.
mov ds, ax                    ; Set the data segment to the A-register.
mov es, ax                    ; Set the extra segment to the A-register.
mov fs, ax                    ; Set the F-segment to the A-register.
mov gs, ax                    ; Set the G-segment to the A-register.
mov ss, ax                    ; Set the G-segment to the A-register.

pop rbx
; High memory size
mov rdi, startupinfo.mem
mov eax, [rbx+8]
stosd
mov eax, 0
stosd
mov eax, [rbx+20]
cmp eax, 1
jb nokernel
mov rdi, kernel64.imageAddr
xor rsi, rsi
mov esi, [rbx+24]
lodsd 
stosd
mov eax, 0
stosd
lodsd
stosd
mov eax, 0
stosd

; Initrd
mov eax, [rbx+20]
cmp eax, 2
jb noinitrd
mov rdi, startupinfo.initrd
xor rsi, rsi
mov esi, [rbx+24]
add esi, 16
lodsd 
stosd
mov eax, 0
stosd
lodsd
stosd
mov eax, 0
stosd

noinitrd:
mov rax, [kernel64.imageEnd]
mov rbx, [startupinfo.initrdEnd]
cmp rax, rbx
ja  .L1
mov rax, rbx
.L1:
add rax, 0xfff
and rax, 0xfffffffffffff000
mov [startupinfo.freepmem_start], rax

; trying to parse and load elf64 kernel
mov rbx, [kernel64.imageAddr]
mov rax, [rbx+24]
mov rdi, kernel64.entry
stosq
mov rsi, [rbx+32]
add rsi, rbx                ; RDX is the Programe Header Offset in memory
add rsi, 8
lodsq                       ; offset in file
stosq
lodsq                       ; virtual memory address
stosq
lodsq                       ; skip p_paddr;
lodsq                       ; p_filesz
stosq
lodsq                       ; p_memsz
stosq

; relocate 64 bit kernel to 0x100000
mov rsi, [kernel64.imageAddr]
add rsi, [kernel64.offset]
mov rdi, 0x100000
mov rcx, [kernel64.filesize]
rep movsb

; fix up stack and parameter pointer,switching to a 8kb kernel stack
mov rbp, kstack
add rbp, VIRTUALBASE
mov rsp, kstack
add rsp, VIRTUALBASE
mov rbx, nokernel
add rbx, VIRTUALBASE
push rax                    ; fake return EIP
mov rax, startupinfo
add rax, VIRTUALBASE
; jmp to the 64 bit kernel, cheers
mov rdi, rax                ; StartupInfo struct
mov rbx, [kernel64.entry]
jmp rbx

nokernel:
jmp $                       ; Enter an infinite loop, to stop the processor

data:
ALIGN 8
gdt64:                           ; Global Descriptor Table (64-bit).
.null: equ $ - gdt64         ; The null descriptor.
dw 0                         ; Limit (low).
dw 0                         ; Base (low).
db 0                         ; Base (middle)
db 0                         ; Access.
db 0                         ; Granularity.
db 0                         ; Base (high).
.code: equ $ - gdt64         ; The code descriptor.
dw 0                         ; Limit (low).
dw 0                         ; Base (low).
db 0                         ; Base (middle)
db 10011010b                 ; Access.
db 00100000b                 ; Granularity.
db 0                         ; Base (high).
.data: equ $ - gdt64         ; The data descriptor.
dw 0                         ; Limit (low).
dw 0                         ; Base (low).
db 0                         ; Base (middle)
db 10010010b                 ; Access.
db 00100000b                 ; Granularity.
db 0                         ; Base (high).
.pointer:                    ; The GDT-pointer.
dw $ - gdt64 - 1             ; Limit.
dq gdt64                     ; Base.

startupinfo:
.mem:
dq 0                        ; Memory info.
.initrd:
dq 0                        ; Initrd address
.initrdEnd:
dq 0                        ; Initrd end
.freepmem_start:
dq 0

kernel64:
.imageAddr:
dq 0
.imageEnd:
dq 0
.entry:
dq 0
.offset:
dq 0
.vma:
dq 0
.filesize:
dq 0
.memsize:
dq 0

ALIGN 4096
times 1024 dq 0
kstack:

end:

