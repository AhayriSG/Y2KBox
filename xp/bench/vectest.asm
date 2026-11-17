; Functional test for 128-bit SSE2 guest memory ops. Prints OK or FAIL<n> on port 0x402.
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
    or  eax, 2
    mov cr0, eax
    mov eax, cr4
    or  eax, 0x600
    mov cr4, eax
    mov word [13*4], gp_handler
    mov word [13*4+2], 0
    mov byte [gpcnt], 0
    ; pattern 0x8000..0x9100
    mov di, 0x8000
    mov cx, 0x1100
    mov al, 3
.f: stosb
    add al, 7
    loop .f

    ; ---- test 1: unaligned loads (incl. crossing 0x9000), aligned-dst stores
    mov bl, '1'
    mov si, 0x8FE0
.t1:
    movdqu xmm0, [si]
    mov di, 0xA100
    movdqu [di], xmm0
    push si
    mov cx, 16
    repe cmpsb
    pop si
    jne fail
    inc si
    cmp si, 0x9011
    jb .t1

    ; ---- test 2: unaligned stores (incl. crossing 0xC000), source aligned
    mov bl, '2'
    movdqa xmm1, [0x8000]
    mov di, 0xBFE0
.t2:
    movdqu [di], xmm1
    push di
    mov si, 0x8000
    mov cx, 16
    repe cmpsb
    pop di
    jne fail
    inc di
    cmp di, 0xC011
    jb .t2

    ; ---- test 3: aligned load/store
    mov bl, '3'
    movdqa xmm2, [0x8010]
    movdqa [0xA200], xmm2
    mov si, 0x8010
    mov di, 0xA200
    mov cx, 16
    repe cmpsb
    jne fail

    ; ---- test 4: misaligned MOVDQA load must raise #GP (4-byte insn skipped by handler)
    mov bl, '4'
    mov si, 0x8001
    movdqa xmm3, [si]
    cmp byte [gpcnt], 1
    jne fail
    ; ---- test 5: misaligned MOVDQA store must raise #GP
    mov bl, '5'
    movdqa [si], xmm3
    cmp byte [gpcnt], 2
    jne fail

    ; ---- test 6: memory-operand ALU op (paddd xmm, [mem]) vs register form
    mov bl, '6'
    movdqa xmm4, [0x8020]
    movdqa xmm5, xmm4
    paddd  xmm4, [0x8030]
    movdqa xmm6, [0x8030]
    paddd  xmm5, xmm6
    movdqa [0xA300], xmm4
    movdqa [0xA310], xmm5
    mov si, 0xA300
    mov di, 0xA310
    mov cx, 16
    repe cmpsb
    jne fail

    ; ---- test 7: MMIO (VGA text RAM 0xB8000) vector load == integer loads
    mov bl, '7'
    mov ax, 0xB800
    mov fs, ax
    xor di, di
    mov cx, 64
    mov al, 0x41
.w: mov [fs:di], al
    inc di
    inc al
    loop .w
    xor si, si
.t7:
    movdqu xmm0, [fs:si]
    movdqu [0xA400], xmm0
    xor bp, bp
.r: mov al, [fs:si+bp]
    mov [0xA500+bp], al
    inc bp
    cmp bp, 16
    jb .r
    push si
    mov si, 0xA400
    mov di, 0xA500
    mov cx, 16
    repe cmpsb
    pop si
    jne fail
    inc si
    cmp si, 33
    jb .t7

    ; ---- test 8: PSHUFD XMM immediate shuffle (reverse dword lanes)
    mov bl, '8'
    movdqa xmm6, [shuffle_input]
    pshufd xmm7, xmm6, 0x1B
    movdqa [0xA600], xmm7
    mov si, 0xA600
    mov di, shuffle_pshufd_expected
    mov cx, 16
    repe cmpsb
    jne fail

    ; ---- test 9: PSHUFLW XMM immediate shuffle (reverse low word lanes)
    mov bl, '9'
    movdqa xmm6, [shuffle_input]
    pshuflw xmm7, xmm6, 0x1B
    movdqa [0xA600], xmm7
    mov si, 0xA600
    mov di, shuffle_pshuflw_expected
    mov cx, 16
    repe cmpsb
    jne fail

    ; ---- test 10: PSHUFHW XMM immediate shuffle (reverse high word lanes)
    mov bl, 'A'
    movdqa xmm6, [shuffle_input]
    pshufhw xmm7, xmm6, 0x1B
    movdqa [0xA600], xmm7
    mov si, 0xA600
    mov di, shuffle_pshufhw_expected
    mov cx, 16
    repe cmpsb
    jne fail

    mov si, ok_msg
    jmp print
fail:
    mov byte [fail_msg+4], bl
    mov si, fail_msg
print:
    mov dx, 0x402
.p: lodsb
    test al, al
    jz .d
    out dx, al
    jmp .p
.d: mov dx, 0xf4
    xor al, al
    out dx, al
.h: hlt
    jmp .h

gp_handler:
    push bp
    mov bp, sp
    add word [bp+2], 4          ; skip the 4-byte faulting movdqa
    inc byte [gpcnt]
    pop bp
    iret
gpcnt: db 0
ok_msg:   db "OK", 10, 0
fail_msg: db "FAIL?", 10, 0
align 16
shuffle_input: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
shuffle_pshufd_expected: db 13,14,15,16,9,10,11,12,5,6,7,8,1,2,3,4
shuffle_pshuflw_expected: db 7,8,5,6,3,4,1,2,9,10,11,12,13,14,15,16
shuffle_pshufhw_expected: db 1,2,3,4,5,6,7,8,15,16,13,14,11,12,9,10
times 510-($-$) db 0
dw 0xAA55
