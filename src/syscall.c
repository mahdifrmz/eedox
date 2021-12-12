#include <syscall.h>
// #include <lock.h>
#include <terminal.h>
#include <multsk.h>
#include <kutil.h>

extern terminal_t glb_term;

void syscall_test();

void syscall_handle(registers *regs)
{
    if (regs->eax == SYSCALL_WRITE)
    {
        char *buffer = (char *)regs->ecx;
        uint32_t len = regs->edx;
        term_print_buffer(&glb_term, buffer, len);
        regs->eax = 0;
    }
    else if (regs->eax == SYSCALL_FORK)
    {
        regs->eax = multsk_fork();
    }
    else if (regs->eax == 0xffffffff)
    {
        syscall_test();
    }
}