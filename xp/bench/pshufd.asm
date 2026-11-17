; Isolated PSHUFD XMM benchmark. Build with -DLOOPS=N; -DEMPTY removes PSHUFD.
bits 16
org 0x7c00
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    mov eax, cr0
    and eax, 0xFFFFFFFB
    or eax, 2
    mov cr0, eax
    mov eax, cr4
    or eax, 0x600
    mov cr4, eax
    movdqa xmm0, [input]
    mov ecx, LOOPS
.loop:
%ifndef EMPTY
    pshufd xmm0, xmm0, 0x1B
%endif
    dec ecx
    jnz .loop
    mov dx, 0xf4
    xor al, al
    out dx, al
.halt:
    hlt
    jmp .halt
align 16
input: dq 0x0706050403020100, 0x0F0E0D0C0B0A0908
times 510-($-$$) db 0
dw 0xAA55
