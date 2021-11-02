global loader                   ; the entry symbol for ELF
global asm_lgdt
global asm_load_segregs
extern kmain
global asm_out
MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
FLAGS        equ 0x0            ; multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                    ; (magic number + checksum + flags should equal 0)
KERNEL_STACK_SIZE equ 4096

section .text:                  ; start of the text (code) section
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum
loader:                         ; the loader label (defined as entry point in linker script)
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    mov ebp, esp
    call kmain
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
asm_load_segregs:
    push ebp
    mov ebp, esp

    mov ax, 0x10
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax
    jmp 0x08:asm_load_segregs_far_jump
asm_load_segregs_far_jump:
    mov esp, ebp
    pop ebp
    ret

section .bss:
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE