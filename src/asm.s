    SEG_CODE equ 0x08
    SEG_DATA equ 0x10

    global asm_out
    global asm_in
    global asm_lgdt
    global asm_lidt

asm_out:
    push ebp
    mov ebp, esp

    mov al, [ebp + 12]    ; move the data to be sent into the al register
    mov dx, [ebp + 8]    ; move the address of the I/O port into the dx register
    out dx, al           ; send the data to the I/O port

    mov esp, ebp
    pop ebp
    ret

asm_in:
    push ebp
    mov ebp, esp

    xor eax,eax

    mov dx, [ebp + 8]    ; move the address of the I/O port into the dx register
    in al, dx           ; send the data to the I/O port

    mov esp, ebp
    pop ebp
    ret
asm_lgdt:

    push ebp
    mov ebp, esp

    lgdt [ebp+8]
    mov ax, SEG_DATA
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    jmp SEG_CODE:_far_jump
    _far_jump:    
    mov esp, ebp
    pop ebp
    ret
asm_lidt:
    push ebp
    mov ebp, esp

    lidt [ebp+8]
    
    mov esp, ebp
    pop ebp
    ret