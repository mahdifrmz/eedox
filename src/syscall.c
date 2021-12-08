#include <syscall.h>
#include <lock.h>
#include <terminal.h>

extern terminal_t glb_term;

void syscall_handle(registers *regs)
{
    if (regs->eax == SYSCALL_WRITE)
    {
        char *buffer = (char *)regs->ecx;
        uint32_t len = regs->edx;
        term_print_buffer(&glb_term, buffer, len);
        regs->eax = 0;
    }
}