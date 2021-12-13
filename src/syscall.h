#ifndef SYSCALL_H
#define SYSCALL_H

#include <idt.h>

#define SYSCALL_READ 2
#define SYSCALL_WRITE 3

#define SYSCALL_FORK 10

void syscall_handle(registers *regs);
void syscall_test();
void syscall_init();

#endif