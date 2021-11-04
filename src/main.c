#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <asm.h>

terminal_t glb_term;
gdtrec glb_gdt_records[3];
idtrec idt_records[256];

typedef struct
{
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no;                                 // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} __attribute__((packed)) registers;

void interrupt_handler(registers regs)
{
    if (regs.int_no != 32)
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
    load_idt_recs(idt_records);
    term_print(&glb_term, "salam\n");
    // init_timer(100);
    // term_clear(&glb_term);
    return 0;
}