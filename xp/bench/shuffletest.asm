; Functional tests for inline XMM immediate shuffle lowering.
bits 16
org 0x7c00
start:
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

    ; PSHUFD: reverse the four dword lanes.
    mov bl, '1'
    movdqa xmm0, [shuffle_input]
    pshufd xmm1, xmm0, 0x1B
    movdqa [result], xmm1
    mov si, result
    mov di, expected_pshufd
    call check16
    jc fail

    ; PSHUFLW: reverse the low four word lanes.
    mov bl, '2'
    movdqa xmm0, [shuffle_input]
    pshuflw xmm1, xmm0, 0x1B
    movdqa [result], xmm1
    mov si, result
    mov di, expected_pshuflw
    call check16
    jc fail

    ; PSHUFHW: reverse the high four word lanes.
    mov bl, '3'
    movdqa xmm0, [shuffle_input]
    pshufhw xmm1, xmm0, 0x1B
    movdqa [result], xmm1
    mov si, result
    mov di, expected_pshufhw
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
shuffle_input: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
expected_pshufd: db 13,14,15,16,9,10,11,12,5,6,7,8,1,2,3,4
expected_pshuflw: db 7,8,5,6,3,4,1,2,9,10,11,12,13,14,15,16
expected_pshufhw: db 1,2,3,4,5,6,7,8,15,16,13,14,11,12,9,10
result: times 16 db 0
ok_msg: db "OK",10,0
fail_msg: db "FAIL?",10,0
times 510-($-$$) db 0
dw 0xAA55
