; Functional coverage for inline legacy XMM PUNPCK, PACK and SHUFPS.
; The boot sector loads the following eight sectors as a second stage.
bits 16
org 0x7c00
boot:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [boot_drive], dl
    mov es, ax
    mov bx, stage2
    mov ah, 0x02
    mov al, 8
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc boot_halt
    jmp 0x0000:stage2
boot_halt:
    hlt
    jmp boot_halt
boot_drive: db 0
times 510-($-$$) db 0
dw 0xaa55

stage2:
    cli
    cld
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

%macro CHECK 5
    mov bl, %1
    movdqa xmm0, [%2]
    movdqa xmm1, [%3]
    %4 xmm0, xmm1
    movdqa [result], xmm0
    mov si, result
    mov di, %5
    call check16
    jc fail
%endmacro

    CHECK '1', unpack8_a, unpack8_b, punpcklbw, expected_punpcklbw
    CHECK '2', unpack8_a, unpack8_b, punpckhbw, expected_punpckhbw
    CHECK '3', unpack16_a, unpack16_b, punpcklwd, expected_punpcklwd
    CHECK '4', unpack16_a, unpack16_b, punpckhwd, expected_punpckhwd
    CHECK '5', unpack32_a, unpack32_b, punpckldq, expected_punpckldq
    CHECK '6', unpack32_a, unpack32_b, punpckhdq, expected_punpckhdq
    CHECK '7', unpack64_a, unpack64_b, punpcklqdq, expected_punpcklqdq
    CHECK '8', unpack64_a, unpack64_b, punpckhqdq, expected_punpckhqdq
    CHECK '9', pack8_a, pack8_b, packsswb, expected_packsswb
    CHECK 'A', pack8_a, pack8_b, packuswb, expected_packuswb
    CHECK 'B', pack32_a, pack32_b, packssdw, expected_packssdw

    mov bl, 'C'
    movdqa xmm0, [shuffle_a]
    movdqa xmm1, [shuffle_b]
    shufps xmm0, xmm1, 0xE4
    movdqa [result], xmm0
    mov si, result
    mov di, expected_shufps
    call check16
    jc fail

    mov si, ok_msg
    jmp print

check16:
    push cx
    mov cx, 16
    repe cmpsb
    pop cx
    jne .bad
    clc
    ret
.bad:
    stc
    ret

fail:
    mov byte [fail_msg+4], bl
    mov si, fail_msg
print:
    mov dx, 0x402
.out:
    lodsb
    test al, al
    jz .done
    out dx, al
    jmp .out
.done:
    mov dx, 0xf4
    xor al, al
    out dx, al
.halt:
    hlt
    jmp .halt

align 16
unpack8_a: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
unpack8_b: db 101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116
expected_punpcklbw: db 1,101,2,102,3,103,4,104,5,105,6,106,7,107,8,108
expected_punpckhbw: db 9,109,10,110,11,111,12,112,13,113,14,114,15,115,16,116

align 16
unpack16_a: dw 1,2,3,4,5,6,7,8
unpack16_b: dw 101,102,103,104,105,106,107,108
expected_punpcklwd: dw 1,101,2,102,3,103,4,104
expected_punpckhwd: dw 5,105,6,106,7,107,8,108

align 16
unpack32_a: dd 1,2,3,4
unpack32_b: dd 101,102,103,104
expected_punpckldq: dd 1,101,2,102
expected_punpckhdq: dd 3,103,4,104

align 16
unpack64_a: dq 1,2
unpack64_b: dq 101,102
expected_punpcklqdq: dq 1,101
expected_punpckhqdq: dq 2,102

align 16
pack8_a: dw -200,-128,-1,0,1,127,128,200
pack8_b: dw -32768,-129,-127,126,255,256,32767,42
expected_packsswb: db 0x80,0x80,0xff,0,1,127,127,127,0x80,0x80,0x81,126,127,127,127,42
expected_packuswb: db 0,0,0,0,1,127,128,200,0,0,0,126,255,255,255,42

align 16
pack32_a: dd -40000,-32768,-1,0,1,32767,32768,40000
pack32_b: dd -2147483648,-32769,-32767,1234,32767,32768,2147483647,42
expected_packssdw: dw -32768,-32768,-1,0,1,32767,32767,32767,-32768,-32768,-32767,1234,32767,32767,32767,42

align 16
shuffle_a: dd 0,1,2,3
shuffle_b: dd 4,5,6,7
expected_shufps: dd 0,1,6,7
result: times 16 db 0
ok_msg: db "OK",10,0
fail_msg: db "FAIL?",10,0

; Total image: boot sector + eight sectors loaded by INT 13h.
times 4608-($-$$) db 0
