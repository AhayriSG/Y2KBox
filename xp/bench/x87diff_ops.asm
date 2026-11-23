; x87 differential test: random operands x all precision/rounding modes x many ops -> one FNV checksum.
; Build: nasm -fbin -DTRIALS=10000 x87diff.asm -o x87diff.img . Run, read the hex checksum on port 0x402.
; The checksum must be IDENTICAL before/after any x87 implementation change.
%ifndef TRIALS
%define TRIALS 10000
%endif
bits 16
org 0x7c00
boot:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    mov [drive], dl
    mov ah, 2
    mov al, 15
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [drive]
    mov bx, 0x7e00
    int 0x13
    jmp 0x0000:0x7e00
drive: db 0
times 510-($-$$) db 0
dw 0xAA55

SEED  equ 0x6000
CHK   equ 0x6004
TRIAL equ 0x6008
CWI   equ 0x600C
OPA   equ 0x6100
OPB   equ 0x6110
OP64  equ 0x6120
OP32  equ 0x6128
CWV   equ 0x612C
MIXP  equ 0x6010
CHKS  equ 0x6300
RES   equ 0x6200            ; 48 bytes: res(10) pad res2(10) pad sw(2) pad out64 out32

main:
    mov eax, cr0
    and eax, 0xFFFFFFFB
    or  eax, 2
    mov cr0, eax
    mov dword [SEED], 0x13572468
    mov dword [CHK], 0x811C9DC5
    mov dword [TRIAL], TRIALS
    mov di, CHKS
    mov cx, 64
    mov eax, 0x811C9DC5
    rep stosd
.trial:
    mov bx, OPA
    call genx80
    mov bx, OPB
    call genx80
    call rnd
    mov [OP64], eax
    call rnd
    mov ecx, eax
    test cl, 3
    jnz .keep64
    and ecx, 0x800FFFFF          ; sometimes: force exponent into the mid range
    or  ecx, 0x3FF00000
    add ecx, 0x00100000
.keep64:
    mov [OP64+4], ecx
    call rnd
    mov [OP32], eax
    test al, 3
    jnz .k32
    and dword [OP32], 0x807FFFFF
    or  dword [OP32], 0x3F800000
.k32:
    mov dword [CWI], 0
.cwloop:
    mov bx, [CWI]
    shl bx, 1
    mov ax, [cwtab + bx]
    mov [CWV], ax

%assign opn 0
%macro DOOP 1
    mov di, RES
    mov cx, 12
    xor eax, eax
    rep stosd
    fninit
    fldcw [CWV]
    fld tword [OPB]
    fld tword [OPA]
    %1
    fnstsw [RES+20]
    fstp tword [RES]
    fstp tword [RES+10]
    mov word [MIXP], CHKS + 4*opn
    call mix
%assign opn opn+1
%endmacro

    DOOP {fadd st0, st1}
    DOOP {fadd st1, st0}
    DOOP {fsub st0, st1}
    DOOP {fsub st1, st0}
    DOOP {fsubr st0, st1}
    DOOP {fsubr st1, st0}
    DOOP {fmul st0, st1}
    DOOP {fmul st1, st0}
    DOOP {fdiv st0, st1}
    DOOP {fdiv st1, st0}
    DOOP {fdivr st0, st1}
    DOOP {fdivr st1, st0}
    DOOP {fsqrt}
    DOOP {fadd qword [OP64]}
    DOOP {fmul qword [OP64]}
    DOOP {fsub qword [OP64]}
    DOOP {fdiv qword [OP64]}
    DOOP {fsubr dword [OP32]}
    DOOP {fadd dword [OP32]}
    DOOP {fdivr dword [OP32]}
    DOOP {fld qword [OP64]}
    DOOP {fld dword [OP32]}
    DOOP {fst qword [RES+32]}
    DOOP {fst dword [RES+40]}
    DOOP {fchs}
    DOOP {fabs}
    DOOP {frndint}
    DOOP {fxch st1}
    DOOP {fcom st1}
    DOOP {fucom st1}
    DOOP {fsubr qword [OP64]}
    DOOP {fdivr qword [OP64]}
    DOOP {fmul dword [OP32]}
    DOOP {fsub dword [OP32]}
    DOOP {fdiv dword [OP32]}
    DOOP {fcom qword [OP64]}
    DOOP {fcom dword [OP32]}
    DOOP {fild dword [OP32]}
    DOOP {fiadd dword [OP32]}
    DOOP {fld st1}
    DOOP {fst st1}
    DOOP {fld1}
    DOOP {fldz}

    ; fcomi st0, st1 (capture EFLAGS CF|PF|ZF in RES+24)
    mov di, RES
    mov cx, 12
    xor eax, eax
    rep stosd
    fninit
    fldcw [CWV]
    fld tword [OPB]
    fld tword [OPA]
    fcomi st0, st1
    pushfd
    pop eax
    and eax, 0x45
    mov [RES+24], eax
    fnstsw [RES+20]
    fstp tword [RES]
    fstp tword [RES+10]
    mov word [MIXP], CHKS + 4*opn
    call mix
%assign opn opn+1

    ; fucomi st0, st1 (capture EFLAGS CF|PF|ZF in RES+24)
    mov di, RES
    mov cx, 12
    xor eax, eax
    rep stosd
    fninit
    fldcw [CWV]
    fld tword [OPB]
    fld tword [OPA]
    fucomi st0, st1
    pushfd
    pop eax
    and eax, 0x45
    mov [RES+24], eax
    fnstsw [RES+20]
    fstp tword [RES]
    fstp tword [RES+10]
    mov word [MIXP], CHKS + 4*opn
    call mix
%assign opn opn+1

    ; chained lazy-SW + fnstsw peek + fnclex + next FP op
    mov di, RES
    mov cx, 12
    xor eax, eax
    rep stosd
    fninit
    fldcw [CWV]
    fld tword [OPB]
    fld tword [OPA]
    fmul st0, st1
    fnstsw [RES+20]
    fadd qword [OP64]
    fnstsw [RES+22]
    fnclex
    fsub dword [OP32]
    fnstsw [RES+24]
    fstp tword [RES]
    fstp tword [RES+10]
    mov word [MIXP], CHKS + 4*opn
    call mix
%assign opn opn+1

    inc word [CWI]
    cmp word [CWI], 12
    jb .cwloop
    dec dword [TRIAL]
    jnz .trial

    mov di, 0
.pl:
    mov eax, [CHKS + di]
    mov cx, 8
.p: rol eax, 4
    push eax
    and al, 15
    add al, '0'
    cmp al, '9'
    jbe .o
    add al, 7
.o: mov dx, 0x402
    out dx, al
    pop eax
    loop .p
    mov al, 10
    out dx, al
    add di, 4
    cmp di, 4*opn
    jb .pl
    mov dx, 0xf4
    xor al, al
    out dx, al
.h: hlt
    jmp .h

rnd:                              ; eax = next LCG value
    mov eax, [SEED]
    imul eax, eax, 1664525
    add eax, 1013904223
    mov [SEED], eax
    shr eax, 0                    ; keep full 32 bits
    ret

genx80:                           ; writes a random 80-bit value at [bx]
    call rnd
    mov [bx], eax
    call rnd
    mov ecx, eax
    mov dl, al
    and dl, 7
    jnz .norm
    and ecx, 0x7FFFFFFF           ; unnormal / denormal-ish: explicit integer bit cleared
    jmp .setm
.norm:
    or  ecx, 0x80000000
.setm:
    test al, 0x38
    jnz .m2
    mov dword [bx], 0             ; power of two
    mov ecx, 0x80000000
.m2:
    mov [bx+4], ecx
    call rnd
    mov cx, ax
    shr ax, 8
    and ax, 15
    cmp ax, 0
    je .e0
    cmp ax, 1
    je .e1
    cmp ax, 2
    je .e7fff
    cmp ax, 3
    je .e7ffe
    cmp ax, 4
    je .edbl_hi
    cmp ax, 5
    je .edbl_lo
    cmp ax, 6
    je .e3fff
    cmp ax, 7
    je .ezero
    ; mid range: 0x3FFF +- up to 255
    and cx, 0x80FF
    mov ax, cx
    and ax, 0x00FF
    sub ax, 128
    add ax, 0x3FFF
    and cx, 0x8000
    or  ax, cx
    jmp .st
.e0:  mov ax, cx
      and ax, 0x8000
      jmp .st
.e1:  mov ax, cx
      and ax, 0x8000
      or  ax, 1
      jmp .st
.e7fff: mov ax, cx
      and ax, 0x8000
      or  ax, 0x7FFF
      jmp .st
.e7ffe: mov ax, cx
      and ax, 0x8000
      or  ax, 0x7FFE
      jmp .st
.edbl_hi: mov ax, cx
      and ax, 0x8001
      or  ax, 0x43FE
      jmp .st
.edbl_lo: mov ax, cx
      and ax, 0x8001
      or  ax, 0x3BFF
      jmp .st
.e3fff: mov ax, cx
      and ax, 0x8000
      or  ax, 0x3FFF
      jmp .st
.ezero: mov dword [bx], 0
      mov dword [bx+4], 0
      mov ax, cx
      and ax, 0x8000
.st:
    mov [bx+8], ax
    ret

mix:
    mov bx, [MIXP]
    mov eax, [bx]
    mov si, RES
    mov cx, 48
.l: movzx edx, byte [si]
    xor eax, edx
    imul eax, eax, 16777619
    inc si
    loop .l
    mov [bx], eax
    ret

cwtab:
    dw 0x003F, 0x043F, 0x083F, 0x0C3F
    dw 0x023F, 0x063F, 0x0A3F, 0x0E3F
    dw 0x033F, 0x073F, 0x0B3F, 0x0F3F
times 8192-($-$$) db 0
