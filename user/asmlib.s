    SYSCALL_EXIT equ 1
    SYSCALL_WRITE equ 3
    SYSCALL_FORK equ 10
    SYSCALL_GETCWD equ 11
    SYSCALL_SETCWD equ 12
    SYSCALL_WAITPID equ 13
    SYSCALL_WAIT equ 14
    
    global write
    global exit
    global getcwd 
    global setcwd
    global $wait
    global waitpid
    global fork 
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
    mov ebx, [ebp+8]
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
$wait:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_WAIT
    mov ebx, [ebp+8]
    int 0x80

    mov esp, ebp
    pop ebp
    ret
waitpid:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_WAITPID
    mov ebx, [ebp+8]
    mov ecx, [ebp+16]
    int 0x80

    mov esp, ebp
    pop ebp
    ret
fork:
    push ebp
    mov ebp, esp

    mov eax, SYSCALL_FORK
    int 0x80

    mov esp, ebp
    pop ebp
    ret