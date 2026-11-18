; Regression test: TCG ring-3 CPL=3 IOPL=0 access to VMware backdoor ports
; 0x5658/0x5659 with a restrictive TSS I/O permission bitmap (all 0xFF).
; Non-exempt port 0x565A must still raise #GP(0). Prints OK or FAIL on 0x402.
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

    ; Build 32-bit TSS at 0x9000 with 8193-byte 0xFF I/O bitmap at +0x68
    mov di, 0x9000
    mov cx, 104 / 2
    rep stosw
    mov word [0x9000 + 4], 0x8000     ; ESP0
    mov word [0x9000 + 8], 0x10       ; SS0 (ring-0 data)
    mov word [0x9000 + 0x66], 0x68    ; IOPB offset = 104
    mov di, 0x9068
    mov cx, 8193
    mov al, 0xff
    rep stosb

    ; Zero IDT at 0x8800 (vectors 0..0x80 -> 129 * 8 = 1032 bytes)
    mov di, 0x8800
    xor ax, ax
    mov cx, 129 * 4
    rep stosw

    ; Vector 13 (#GP): 32-bit interrupt gate, selector 0x08, DPL=0
    mov eax, gp32_handler
    mov [0x8800 + 13 * 8], ax
    mov word [0x8800 + 13 * 8 + 2], 0x0008
    mov word [0x8800 + 13 * 8 + 4], 0x8e00
    shr eax, 16
    mov [0x8800 + 13 * 8 + 6], ax

    ; Vector 0x80 (exit): 32-bit interrupt gate, selector 0x08, DPL=3
    mov eax, exit32_handler
    mov [0x8800 + 0x80 * 8], ax
    mov word [0x8800 + 0x80 * 8 + 2], 0x0008
    mov word [0x8800 + 0x80 * 8 + 4], 0xee00
    shr eax, 16
    mov [0x8800 + 0x80 * 8 + 6], ax

    lgdt [gdt_desc]
    lidt [idt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm32_start

bits 32
pm32_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x8000

    mov ax, 0x28
    ltr ax

    ; Transition to CPL=3, IOPL=0 via iretd frame: SS, ESP, EFLAGS, CS, EIP
    push dword 0x23
    push dword 0x8400
    push dword 0x0002
    push dword 0x1b
    push dword ring3_start
    iretd

ring3_start:
    mov ax, 0x23
    mov ds, ax
    mov es, ax

    ; 1) Ring-3 IN on exempt 0x5658 (GETVERSION) must succeed without #GP
    mov eax, 0x564D5868
    xor ebx, ebx
    mov ecx, 10
    mov dx, 0x5658
    in eax, dx
    cmp ebx, 0x564D5868
    jne .fail
    cmp dword [gp_count], 0
    jne .fail

    ; 2) Ring-3 OUT on exempt 0x5659 must succeed without #GP
    mov eax, 0x564D5868
    xor ebx, ebx
    mov ecx, 10
    mov dx, 0x5659
    out dx, eax
    cmp dword [gp_count], 0
    jne .fail

    ; 3) Ring-3 IN on non-exempt 0x565A must trap #GP(0) (1-byte opcode 0xED)
    mov dx, 0x565A
    in eax, dx
    cmp dword [gp_count], 1
    jne .fail

    mov al, 1
    int 0x80
.fail:
    xor al, al
    int 0x80

gp32_handler:
    push eax
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    inc dword [gp_count]
    add dword [esp + 12], 1     ; stack: [saved_ds, saved_eax, err_code, EIP, ...]
    pop eax
    mov ds, ax
    pop eax
    add esp, 4                  ; discard #GP error code
    iretd

exit32_handler:
    mov dx, 0x402
    test al, al
    jz .bad
    mov al, 'O'
    out dx, al
    mov al, 'K'
    out dx, al
    jmp .done
.bad:
    mov al, 'F'
    out dx, al
    mov al, 'A'
    out dx, al
    mov al, 'I'
    out dx, al
    mov al, 'L'
    out dx, al
.done:
    mov al, 10
    out dx, al
    mov dx, 0xf4
    xor al, al
    out dx, al
.hang:
    hlt
    jmp .hang

gp_count:
    dd 0

align 8
gdt:
    dq 0x0000000000000000       ; 0x00 null
    dq 0x00cf9a000000ffff       ; 0x08 ring-0 code
    dq 0x00cf92000000ffff       ; 0x10 ring-0 data
    dq 0x00cffa000000ffff       ; 0x18 ring-3 code
    dq 0x00cff2000000ffff       ; 0x20 ring-3 data
    ; 0x28: 32-bit available TSS at 0x9000, limit 0x2068 (base 0x9000 -> bytes [0x9000..0xb068])
    dw 0x2068, 0x9000
    db 0x00, 0x89, 0x00, 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt - 1
    dd gdt

idt_desc:
    dw 129 * 8 - 1
    dd 0x8800

times 510-($-$$) db 0
dw 0xaa55
