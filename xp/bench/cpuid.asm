; prints CPUID(1) EDX:ECX and family/model, then exits. Build: nasm -fbin cpuid.asm -o cpuid.img
bits 16
org 0x7c00
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    mov eax, 1
    cpuid
    mov [saved_eax], eax
    mov [saved_edx], edx
    mov [saved_ecx], ecx
    mov si, tag_e
    call puts
    mov eax, [saved_eax]
    call hex32
    mov si, tag_d
    call puts
    mov eax, [saved_edx]
    call hex32
    mov si, tag_c
    call puts
    mov eax, [saved_ecx]
    call hex32
    mov al, 10
    call putc
    mov dx, 0xf4
    xor al, al
    out dx, al
.h: hlt
    jmp .h
putc:
    push dx
    mov dx, 0x402
    out dx, al
    pop dx
    ret
puts:
    lodsb
    test al, al
    jz .r
    call putc
    jmp puts
.r: ret
hex32:
    mov cx, 8
.l: rol eax, 4
    push eax
    and al, 15
    add al, '0'
    cmp al, '9'
    jbe .o
    add al, 7
.o: call putc
    pop eax
    loop .l
    ret
tag_e: db "EAX=", 0
tag_d: db " EDX=", 0
tag_c: db " ECX=", 0
saved_eax: dd 0
saved_edx: dd 0
saved_ecx: dd 0
times 510-($-$$) db 0
dw 0xAA55
