    SEG_CODE equ 0x08
    SEG_DATA equ 0x10

    global asm_outb
    global asm_outw
    global asm_inb
    global asm_outsw
    global asm_insw
    global asm_lgdt
    global asm_lidt
    global asm_cli
    global asm_sti

    global switch_page_directory
    global paging_physcpy
    global asm_usermode

    global asm_get_eip
    global asm_get_ebp
    global asm_get_esp
    global asm_task_switch
    global asm_set_sps 
    global asm_flush_TLB
    global asm_flush_tss
    global asm_get_cr2
    global task_sleep

    extern eip_buffer
    extern ebp_buffer
    extern esp_buffer
    extern current_page_directory
    extern task_switch
asm_outb:
    push ebp
    mov ebp, esp

    mov al, [ebp + 12]    ; move the data to be sent into the al register
    mov dx, [ebp + 8]    ; move the address of the I/O port into the dx register
    out dx, al           ; send the data to the I/O port

    mov esp, ebp
    pop ebp
    ret

asm_outw:
    push ebp
    mov ebp, esp

    mov ax, [ebp + 12]    ; move the data to be sent into the al register
    mov dx, [ebp + 8]    ; move the address of the I/O port into the dx register
    out dx, ax           ; send the data to the I/O port

    mov esp, ebp
    pop ebp
    ret

asm_inb:
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
    sti
    
    mov esp, ebp
    pop ebp
    ret
switch_page_directory:
    push ebp
    mov ebp, esp

    mov eax, [esp + 8]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret
paging_physcpy:

    push ebp
    mov ebp, esp
    push ebx
    pushf
    
    mov ebx, [ebp + 8]
    mov ecx, [ebp + 12]
    mov edx, 1024
    
    mov eax, cr0
    and eax, 0x7fffffff
    mov cr0, eax

    _paging_physcpy_loop:
    mov eax, [ebx]
    mov [ecx], eax
    add ebx, 4
    add ecx, 4
    dec edx
    jnz _paging_physcpy_loop
    
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    popf
    pop ebx
    mov esp, ebp
    pop ebp

asm_get_eip:
    mov eax, [esp]
    ret
asm_get_esp:
    mov eax, esp
    add eax, 0x4
    ret
asm_get_ebp:
    mov eax, ebp    
    ret
asm_task_switch:
    add esp, 4
    mov eax, [esp_buffer]
    mov esp, eax
    mov eax, [ebp_buffer]
    mov ebp, eax
    mov eax, [current_page_directory]
    add eax, 0x1000
    mov cr3, eax
    mov eax, 0xffffffff
    mov ecx, [eip_buffer]
    jmp ecx
asm_cli:
    cli
    ret
asm_sti:
    sti
    ret
asm_set_sps:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    pop ecx
    mov ebp, eax
    mov esp, ebx
    jmp ecx
asm_flush_TLB:
    mov eax, cr3
    mov cr3, eax
    ret
asm_flush_tss:
    mov ax, 0x2b
    ltr ax
    ret

asm_usermode:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop ebx ; the return address (discarded)
    pop ebx ; user program address

    mov eax, esp
    push 0x23 ; stack segment
    push eax  ; esp
    pushf     ; flags

    ; set int flag
    pop eax        
    or eax, 0x200
    push eax

    push 0x1b ;
    push ebx

    iret

asm_get_cr2:
    mov eax, cr2
    ret

asm_outsw:
    push ebp
    mov ebp, esp

    mov dx, [ebp+8]   ; port
    mov esi, [ebp+12] ; address
    mov ecx, [ebp+16] ; count

    cld
    rep outsw

    mov esp, ebp
    pop ebp 
    ret

asm_insw:
    push ebp
    mov ebp, esp

    mov dx, [ebp+8]   ; port
    mov edi, [ebp+12] ; address
    mov ecx, [ebp+16] ; count

    cld
    rep insw

    mov esp, ebp
    pop ebp 
    ret

task_sleep:
    push ebp
    mov ebp, esp

    pusha
    pushf
    push 0x00000001
    call task_switch
    add esp, 4
    popf
    popa

    mov esp, ebp
    pop ebp 
    ret
