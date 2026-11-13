; SSE2 microbenchmark boot sector (real mode). Build: nasm -fbin -DLOOPS=N [-DMEM] sse2.asm
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
    and eax, 0xFFFFFFFB        ; EM = 0
    or  eax, 2                 ; MP = 1
    mov cr0, eax
    mov eax, cr4
    or  eax, 0x600             ; OSFXSR | OSXMMEXCPT
    mov cr4, eax
    mov bx, 0x8000             ; 16-byte aligned scratch
    pcmpeqb xmm1, xmm1
    pxor    xmm0, xmm0
    pcmpeqb xmm2, xmm2
    pxor    xmm7, xmm7
    movdqa  [bx], xmm1
    mov ecx, LOOPS
.l:
%ifdef MEM
    paddd   xmm0, [bx]
    movdqa  [bx+16], xmm0
    movdqu  xmm3, [bx+16]
%endif
    paddd   xmm0, xmm1
    pmullw  xmm2, xmm0
    pxor    xmm3, xmm2
    pshufd  xmm4, xmm3, 0x1b
    pcmpeqb xmm5, xmm4
    pand    xmm6, xmm5
    por     xmm0, xmm6
    psrld   xmm1, 1
    paddw   xmm7, xmm1
    psubb   xmm2, xmm7
    dec ecx
    jnz .l
    mov dx, 0xf4
    xor al, al
    out dx, al
.h: hlt
    jmp .h
times 510-($-$$) db 0
dw 0xAA55
