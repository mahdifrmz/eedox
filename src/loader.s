global loader                   ; the entry symbol for ELF
global asm_lgdt
global asm_lidt
    extern kmain
    
    MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
    FLAGS        equ 0x0            ; multiboot flags
    CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                    ; (magic number + checksum + flags should equal 0)
    SEG_CODE equ 0x08
    SEG_DATA equ 0x10

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
    mov ebp, esp
    call kmain
    jmp $