#include <syscall.h>
#include <terminal.h>
#include <multsk.h>
#include <kutil.h>
#include <kstring.h>
#include <lock.h>

extern terminal_t glb_term;
extern task_t *reader_task;
extern krwlock reader_lock;
extern kstring_t input_buffer;

void syscall_test();

void syscall_handle(registers *regs)
{
    if (regs->eax == SYSCALL_WRITE)
    {
        const char *buffer = (char *)regs->ecx;
        uint32_t len = regs->edx;
        term_print_buffer(&glb_term, buffer, len);
        regs->eax = 0;
    }
    if (regs->eax == SYSCALL_READ)
    {
        krwlock_write(&reader_lock);
        char *buffer = (char *)regs->ecx;
        uint32_t len = regs->edx;
        while (1)
        {
            uint32_t count = min(len, input_buffer.size);
            memcpy(buffer, kstring_str(&input_buffer), count);
            kstring_erase(&input_buffer, 0, count);
            len -= count;
            buffer += count;
            if (!len)
            {
                break;
            }
            reader_task = multsk_curtask();
            multsk_sleep();
        };
        regs->eax = 0;
        krwlock_release(&reader_lock);
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