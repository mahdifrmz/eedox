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

terminal_t glb_term;
gdtrec glb_gdt_records[6];
idtrec idt_records[256];
heap_t kernel_heap;
extern uint32_t end;
tss_rec tss_entry;

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

typedef struct
{
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;                       // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} __attribute__((packed)) registers;

void interrupt_handler(registers regs)
{
    if (regs.int_no == 33)
    {
        uint8_t scancode = asm_in(0x60);
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
    else if (regs.int_no == 14)
    {
        kpanic("paging fault");
    }
    else if (regs.int_no == 32)
    {
        if (multsk_flag)
        {
            multsk_switch();
        }
    }
    else
    {
        kprintf("interrupt %u\n", regs.int_no);
    }
}

void irq_handler(registers regs)
{
    if (regs.int_no >= 8)
    {
        asm_out(0xA0, 0x20);
    }
    asm_out(0x20, 0x20);
    regs.int_no += 32;
    interrupt_handler(regs);
}

void init_timer(uint32_t frequency)
{
    uint16_t divisor = 1193180 / frequency;
    asm_out(0x43, 0x36);
    uint8_t l = (uint8_t)(divisor % 256);
    uint8_t h = (uint8_t)(divisor / 256);
    asm_out(0x40, l);
    asm_out(0x40, h);
}

void move_stack(uint32_t new_address, uint32_t old_address, uint32_t size)
{

    for (uint32_t i = new_address; i >= new_address - size; i -= 0x1000)
    {
        alloc_frame(get_page(i, 0, current_page_directory), 1, 0);
    }
    asm_flush_TLB();
    uint32_t offset = new_address - old_address;
    uint32_t old_ebp = asm_get_ebp();
    uint32_t old_esp = asm_get_esp();
    uint32_t new_ebp = old_ebp + offset;
    uint32_t new_esp = old_esp + offset;
    memcpy((void *)new_esp, (void *)old_esp, old_address - old_esp);
    for (uint32_t i = new_address; i > new_address - size; i -= 4)
    {
        uint32_t value = *(uint32_t *)i;
        if (value <= old_address && value >= old_esp)
        {
            *(uint32_t *)i += offset;
        }
    }
    asm_set_sps(new_ebp, new_esp);
}

int kmain(uint32_t stack_address)
{
    term_init(&glb_term);
    term_fg(&glb_term);
    load_gdt_recs(glb_gdt_records, &tss_entry);
    load_idt_recs(idt_records, interrupt_handler, irq_handler);
    heap_init(&kernel_heap, &end, 0x1004000, 0x4000, 1, 1);
    paging_init();
    move_stack(0xC0000000, stack_address, 0x2000);
    multsk_init();
    init_timer(100);
    multsk_fork();
    kprintf("hello from thread %u\n", multk_getpid());
    return 0;
}