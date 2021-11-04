#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <asm.h>

terminal_t glb_term;
gdtrec glb_gdt_records[3];
idtrec idt_records[256];

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
    uint32_t int_no;                                 // Interrupt number and error code (if applicable)
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
                term_write_char(&glb_term, ch);
            }
        }
    }
    else if (regs.int_no != 32)
    {
        term_print(&glb_term, "interrupt ");
        term_print_dword_dec(&glb_term, regs.int_no);
        term_print(&glb_term, "\n");
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

int kmain()
{
    term_init(&glb_term);
    term_fg(&glb_term);
    load_gdt_recs(glb_gdt_records);
    load_idt_recs(idt_records, interrupt_handler, irq_handler);
    term_print(&glb_term, "salam\n");
    // init_timer(100);
    // term_clear(&glb_term);
    return 0;
}