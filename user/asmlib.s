    SYSCALL_WRITE equ 3
    SYSCALL_EXIT equ 1
    
    global write
    global exit 
section .text
write:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_WRITE
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    mov edx, [ebp+16]
    int 0x80

    mov esp, ebp
    pop ebp
    ret

exit:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_EXIT
    int 0x80

    mov esp, ebp
    pop ebp
    ret