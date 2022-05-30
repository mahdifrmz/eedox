%macro SYSCALL_1R 2
global %1
%1:
    push ebp
    mov ebp, esp

    mov eax, %2
    int 0x80

    mov esp ,ebp
    pop ebp
    ret
%endmacro

%macro SYSCALL_2R 2
global %1
%1:
    push ebp
    mov ebp, esp

    mov eax, %2
    mov ebx, [ebp+8]
    int 0x80

    mov esp ,ebp
    pop ebp
    ret
%endmacro

%macro SYSCALL_3R 2
global %1
%1:
    push ebp
    mov ebp, esp

    mov eax, %2
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    int 0x80

    mov esp ,ebp
    pop ebp
    ret
%endmacro

%macro SYSCALL_4R 2
global %1
%1:
    push ebp
    mov ebp, esp

    mov eax, %2
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    mov edx, [ebp+16]
    int 0x80

    mov esp ,ebp
    pop ebp
    ret
%endmacro


section .text
    SYSCALL_2R exit, 1
    SYSCALL_4R read, 2
    SYSCALL_4R write, 3
    SYSCALL_3R open, 4
    SYSCALL_2R close, 5
    SYSCALL_2R opendir, 6
    SYSCALL_3R readdir, 7
    SYSCALL_3R exec, 9
    SYSCALL_1R fork, 10
    SYSCALL_2R getcwd, 11
    SYSCALL_2R setcwd, 12
    SYSCALL_3R waitpid, 13
    SYSCALL_2R $wait, 14
    SYSCALL_1R getpid, 15