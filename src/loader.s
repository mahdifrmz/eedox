global loader                   ; the entry symbol for ELF
global asm_lgdt
global asm_lidt
global asm_load_segregs
extern kmain
extern irq_handler
extern term_print_dword_dec
extern term_print_endl
extern term_print_flag
extern interrupt_handler
global asm_out
MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
FLAGS        equ 0x0            ; multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                    ; (magic number + checksum + flags should equal 0)
KERNEL_STACK_SIZE equ 4096

SEG_CODE equ 0x08
SEG_DATA equ 0x10

[BITS 32]                       ; All instructions should be 32-bit.
[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

                  ; start of the text (code) section
mboot:
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum
    
    dd  mboot                     ; Location of this descriptor
    dd  code                      ; Start of kernel '.text' (code) section.
    dd  bss                       ; End of kernel '.data' section.
    dd  end                       ; End of kernel.
    dd  loader

section .text:                     ; Kernel entry point (initial EIP).
loader:                         ; the loader label (defined as entry point in linker script)
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    mov ebp, esp
    call kmain
    sti
    ; int 17
    jmp $
asm_out:
    push ebp
    mov ebp, esp

    push eax
    push edx
    mov al, [ebp + 12]    ; move the data to be sent into the al register
    mov dx, [ebp + 8]    ; move the address of the I/O port into the dx register
    out dx, al           ; send the data to the I/O port
    pop edx
    pop eax

    mov esp, ebp
    pop ebp
    ret
asm_lgdt:

    push ebp
    mov ebp, esp

    lgdt [ebp+8]
    
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
asm_load_segregs:
    push ebp
    mov ebp, esp

    mov ax, SEG_DATA
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    jmp SEG_CODE:asm_load_segregs_far_jump
asm_load_segregs_far_jump:
    mov esp, ebp
    pop ebp
    ret

%macro IRQ 1
    global each_irq_handler_%1
    each_irq_handler_%1
        cli
        push    dword %1                    ; push the interrupt number
        jmp     common_irq_handler    ; jump to the common handler
%endmacro

%macro no_error_code_interrupt_handler 1
    global interrupt_handler_%1
    interrupt_handler_%1:
        cli
        push    dword %1                    ; push the interrupt number
        jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler 1
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
    
    call interrupt_handler

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

    no_error_code_interrupt_handler 0
    no_error_code_interrupt_handler 1
    no_error_code_interrupt_handler 2
    no_error_code_interrupt_handler 3
    no_error_code_interrupt_handler 4
    no_error_code_interrupt_handler 5
    no_error_code_interrupt_handler 6
    no_error_code_interrupt_handler 7
    error_code_interrupt_handler    8
    no_error_code_interrupt_handler 9
    error_code_interrupt_handler    10
    error_code_interrupt_handler    11
    no_error_code_interrupt_handler 12
    error_code_interrupt_handler    13
    error_code_interrupt_handler    14
    no_error_code_interrupt_handler 15
    no_error_code_interrupt_handler 16
    no_error_code_interrupt_handler 17
    no_error_code_interrupt_handler 18

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

    call irq_handler

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

section .bss:
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE