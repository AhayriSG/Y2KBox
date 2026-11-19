; Isolated non-popping FST m32 benchmark. The guest ST0 value remains constant.
; Compare the same QEMU binary with XPQEMU_NO_HX87=1 and with the fast path enabled.
%ifndef LOOPS
%define LOOPS 5000000
%endif
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
    fld qword [source]

    mov ebp, LOOPS
.loop:
    fst dword [dest]
    dec ebp
    jnz .loop

    mov dx, 0xf4
    xor al, al
    out dx, al
.halt:
    hlt
    jmp .halt

source: dq 1.2345678901234567
dest:   dd 0
times 510-($-$$) db 0
