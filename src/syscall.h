#ifndef SYSCALL_H
#define SYSCALL_H

#include <idt.h>

#define SYSCALL_READ 2
#define SYSCALL_WRITE 3

void syscall_handle(registers *regs);

#endif