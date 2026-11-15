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
    fninit
    mov eax, 0x12345678
    mov ebx, 0x9abcdef1
    mov ecx, 7
    mov edx, 3
    mov esi, 5
    mov edi, 0x8000

    mov ebp, 30000000
.l:
    add eax,ebx
    xor ebx,eax
    shl eax,3
    sub ecx,eax
    and edx,0xff
    or edx,ecx
    adc esi,eax
    ror ebx,5
    not ecx
    neg edx

    dec ebp
    jnz .l
    mov dx, 0xf4
    xor al, al
    out dx, al
.h: hlt
    jmp .h

fa: dq 1.0000001
fb: dq 0.9999999
times 510-($-$$) db 0
dw 0xAA55
