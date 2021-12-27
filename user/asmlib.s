    SYSCALL_WRITE equ 3
    SYSCALL_GETCWD equ 11
    SYSCALL_SETCWD equ 12
    SYSCALL_EXIT equ 1
    
    global write
    global exit
    global getcwd 
    global setcwd 
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
getcwd:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_GETCWD
    mov ebx, [ebp+8]
    int 0x80

    mov esp, ebp
    pop ebp
    ret
setcwd:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_SETCWD
    mov ebx, [ebp+8]
    int 0x80

    mov esp, ebp
    pop ebp
    ret
