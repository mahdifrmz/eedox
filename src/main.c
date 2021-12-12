#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <asm.h>
#include <kheap.h>
#include <paging.h>
#include <bitset.h>
#include <kstring.h>
#include <kutil.h>
#include <vec.h>
#include <multsk.h>
#include <kqueue.h>
// #include <ide.h>
#include <syscall.h>

terminal_t glb_term;
gdtrec glb_gdt_records[6];
idtrec idt_records[256];
heap_t kernel_heap;
extern uint32_t end;
tss_rec tss_entry;

uint32_t kernel_memory_end;
int32_t user_write(uint32_t fd, void *buffer, uint32_t length);

extern uint32_t inldr_end;
extern uint32_t inldr_start;

unsigned char kbchars[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
    '9', '0', '-', '=', '\b',                         /* Backspace */
    0,                                                /* Tab */
    'q', 'w', 'e', 'r',                               /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* Enter key */
    0,                                                /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`', 0,                                     /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',               /* 49 */
    'm', ',', '.', '/', 0,                            /* Right shift */
    '*',
    0,   /* Alt */
    ' ', /* Space bar */
    0,   /* Caps lock */
    0,   /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};

void interrupt_handler(registers *regs)
{
    if (regs->int_no == 33)
    {
        uint8_t scancode = asm_inb(0x60);
        if (scancode < 128)
        {
            char ch = kbchars[scancode];
            if (ch != 0)
            {
                if (ch == '\b')
                {
                    term_backspace(&glb_term);
                }
                else
                {
                    term_write_char(&glb_term, ch);
                }
            }
        }
    }
    else if (regs->int_no == 14)
    {
        const char *present = !(regs->err_code & 0x1) ? "present " : ""; // Page not present
        const char *rw = regs->err_code & 0x2 ? "read-only " : "";       // Write operation?
        const char *us = regs->err_code & 0x4 ? "user-mode " : "";       // Processor was in user-mode?
        const char *reserved = regs->err_code & 0x8 ? "reserved " : "";  // Overwritten CPU-reserved bits of page entry?
        const char *fetch = regs->err_code & 0x10 ? "fetch " : "";
        kpanic("paging fault [%x] ( %s%s%s%s%s) ", asm_get_cr2(), present, rw, us, reserved, fetch);
    }
    else if (regs->int_no == 13)
    {
        kpanic("general protection fault ( code = %x )", regs->err_code);
    }
    else if (regs->int_no == 32)
    {
        if (multsk_flag)
        {
            multsk_switch();
        }
    }
    else if (regs->int_no == 46)
    {
        // ata_ihandler();
    }
    else if (regs->int_no == 0x80)
    {
        syscall_handle(regs);
    }
    else
    {
        kprintf("interrupt %u\n", regs->int_no);
    }
}

void irq_handler(registers *regs)
{
    if (regs->int_no >= 8)
    {
        asm_outb(0xA0, 0x20);
    }
    asm_outb(0x20, 0x20);
    regs->int_no += 32;
    interrupt_handler(regs);
}

void init_timer(uint32_t frequency)
{
    uint16_t divisor = 1193180 / frequency;
    asm_outb(0x43, 0x36);
    uint8_t l = (uint8_t)(divisor % 256);
    uint8_t h = (uint8_t)(divisor / 256);
    asm_outb(0x40, l);
    asm_outb(0x40, h);
}

void stack_init()
{
    kernel_stack_ptr = 0xC0000000;
    user_stack_ptr = kernel_stack_ptr + KERNEL_STACK_SIZE;
    for (uint32_t i = 0; i < KERNEL_STACK_SIZE; i += 0x1000)
    {
        alloc_frame(get_page(kernel_stack_ptr + i, 0, current_page_directory), 1, 0);
    }
    for (uint32_t i = 0; i < USER_STACK_SIZE; i += 0x1000)
    {
        alloc_frame(get_page(user_stack_ptr + i, 0, current_page_directory), 1, 0);
    }
}

void *load_indlr()
{
    alloc_frame(get_page(kernel_memory_end, 0, current_page_directory), 1, 0);
    memcpy((void *)kernel_memory_end, &inldr_start, (uint32_t)&inldr_end - (uint32_t)&inldr_start);
    return (void *)kernel_memory_end;
}

void syscall_test()
{
}

void kinit()
{
    term_init(&glb_term);
    term_fg(&glb_term);
    load_gdt_recs(glb_gdt_records, &tss_entry);
    load_idt_recs(idt_records, interrupt_handler, irq_handler);
    uint32_t heap_effective_size = 0x1000000;
    uint32_t heap_index_size = 0x4000;
    uint32_t heap_total_size = heap_effective_size + heap_index_size;
    heap_init(&kernel_heap, &end, heap_total_size, heap_index_size, 1, 1);
    kernel_memory_end = (uint32_t)kernel_heap.start + heap_total_size;
    uint32_t table_space = 0x400000;
    if (kernel_memory_end % table_space != 0)
    {
        kernel_memory_end /= table_space;
        kernel_memory_end *= table_space;
        kernel_memory_end += table_space;
    }
    paging_init();
    stack_init();
}

void kmain()
{
    init_timer(100);
    multsk_init();
    // multsk_fork();
    asm_usermode(load_indlr());
    // kprintf("I am %u\n", multk_getpid());
}