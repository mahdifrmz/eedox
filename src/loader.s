    global loader                   ; the entry symbol for ELF
    global asm_lgdt
    global asm_lidt

    extern kinit
    extern kmain
    extern user_stack_ptr

    global user_write
    global inldr_start
    global inldr_end
    
    MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
    FLAGS        equ 0x0            ; multiboot flags
    CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                    ; (magic number + checksum + flags should equal 0)
    SEG_CODE equ 0x08
    SEG_DATA equ 0x10
    INITIAL_STACK_SIZE equ 0x2000
    USER_STACK_SIZE equ 0x2000

[BITS 32]                       ; All instructions should be 32-bit.
[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

                  ; start of the text (code) section

section .mbhead
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
    mov esp, initial_stack + INITIAL_STACK_SIZE
    mov ebp, esp
    call kinit
    mov esp, [user_stack_ptr]
    add esp, USER_STACK_SIZE
    mov ebp, esp
    call kmain
    jmp $
inldr_start:
    ; mov eax, 10
    ; int 0x80
    ; push 'zdrs'
    push 0
    push 0
    push "in: "

    mov eax, 2
    mov ecx, esp
    add ecx, 4
    mov edx, 8
    int 0x80

    mov eax, 3
    mov ecx, esp
    mov edx, 12
    int 0x80

    ; mov eax, 0xffffffff
    ; int 0x80
    jmp $
inldr_end:
user_write:
    push ebp
    mov ebp, esp

    mov eax, 3
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    mov edx, [ebp+16]
    int 0x80

    mov esp, ebp
    pop ebp 
    ret

section .bss:
    initial_stack resb INITIAL_STACK_SIZE