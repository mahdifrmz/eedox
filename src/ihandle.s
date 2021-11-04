    SEG_CODE equ 0x08
    SEG_DATA equ 0x10
    global set_interrupt_handler
    global set_irq_handler

section .data:
    interrupt_handler dd 0x0 
    irq_handler dd 0x0

%macro IRQ 1
    global each_irq_handler_%1
    each_irq_handler_%1
        cli
        push    dword %1                    ; push the interrupt number
        jmp     common_irq_handler    ; jump to the common handler
%endmacro

%macro NERR_INT_HANLDLER 1
    global interrupt_handler_%1
    interrupt_handler_%1:
        cli
        push    dword %1                    ; push the interrupt number
        jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro ERR_INT_HANLDLER 1
    global interrupt_handler_%1
    interrupt_handler_%1:
        cli
        push    dword %1                    ; push the interrupt number
        jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

common_interrupt_handler:

    pusha
    mov ax, ds
    push eax
    mov ax, SEG_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    call [interrupt_handler]

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    popa
    
    add esp, 4
    sti
    iret

    NERR_INT_HANLDLER 0
    NERR_INT_HANLDLER 1
    NERR_INT_HANLDLER 2
    NERR_INT_HANLDLER 3
    NERR_INT_HANLDLER 4
    NERR_INT_HANLDLER 5
    NERR_INT_HANLDLER 6
    NERR_INT_HANLDLER 7
    ERR_INT_HANLDLER    8
    NERR_INT_HANLDLER 9
    ERR_INT_HANLDLER    10
    ERR_INT_HANLDLER    11
    NERR_INT_HANLDLER 12
    ERR_INT_HANLDLER    13
    ERR_INT_HANLDLER    14
    NERR_INT_HANLDLER 15
    NERR_INT_HANLDLER 16
    NERR_INT_HANLDLER 17
    NERR_INT_HANLDLER 18

common_irq_handler:

    pusha
    mov ax, ds
    push eax
    mov ax, SEG_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call [irq_handler]

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    popa
    
    add esp, 4
    sti
    iret

    IRQ 0
    IRQ 1
    IRQ 2
    IRQ 3
    IRQ 4
    IRQ 5
    IRQ 6
    IRQ 7
    IRQ 8
    IRQ 9
    IRQ 10
    IRQ 11
    IRQ 12
    IRQ 13
    IRQ 14
    IRQ 15

set_interrupt_handler:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov [interrupt_handler], eax
    
    mov esp, ebp
    pop ebp
    ret
set_irq_handler:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov [irq_handler], eax
    
    mov esp, ebp
    pop ebp
    ret