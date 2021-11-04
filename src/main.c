#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <asm.h>

terminal_t glb_term;
gdtrec glb_gdt_records[3];
idtrec idt_records[256];

void interrupt_handler_0();
void interrupt_handler_1();
void interrupt_handler_2();
void interrupt_handler_3();
void interrupt_handler_4();
void interrupt_handler_5();
void interrupt_handler_6();
void interrupt_handler_7();
void interrupt_handler_8();
void interrupt_handler_9();
void interrupt_handler_10();
void interrupt_handler_11();
void interrupt_handler_12();
void interrupt_handler_13();
void interrupt_handler_14();
void interrupt_handler_15();
void interrupt_handler_16();
void interrupt_handler_17();
void interrupt_handler_18();

void each_irq_handler_0();
void each_irq_handler_1();
void each_irq_handler_2();
void each_irq_handler_3();
void each_irq_handler_4();
void each_irq_handler_5();
void each_irq_handler_6();
void each_irq_handler_7();
void each_irq_handler_8();
void each_irq_handler_9();
void each_irq_handler_10();
void each_irq_handler_11();
void each_irq_handler_12();
void each_irq_handler_13();
void each_irq_handler_14();
void each_irq_handler_15();

typedef struct
{
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no;                                 // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} __attribute__((packed)) registers;

typedef enum
{
    igate_type_interrupt = 0x8E,
    igate_type_trap = 0x8F,
    igate_type_task = 0x85,
} igate_type;

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

idtrec create_idt_rec(void *handler, igate_type type)
{
    idtrec rec;
    rec.zero = 0;
    rec.type_attr = type;
    rec.selector = 0x08;
    rec.offset_1 = (int)handler % (UINT16_MAX + 1);
    rec.offset_2 = (int)handler / (UINT16_MAX + 1);
    return rec;
}

void load_idt_recs()
{
    idt_records[0] = create_idt_rec(interrupt_handler_0, igate_type_trap);
    idt_records[1] = create_idt_rec(interrupt_handler_1, igate_type_trap);
    idt_records[2] = create_idt_rec(interrupt_handler_2, igate_type_trap);
    idt_records[3] = create_idt_rec(interrupt_handler_3, igate_type_trap);
    idt_records[4] = create_idt_rec(interrupt_handler_4, igate_type_trap);
    idt_records[5] = create_idt_rec(interrupt_handler_5, igate_type_trap);
    idt_records[6] = create_idt_rec(interrupt_handler_6, igate_type_trap);
    idt_records[7] = create_idt_rec(interrupt_handler_7, igate_type_trap);
    idt_records[8] = create_idt_rec(interrupt_handler_8, igate_type_trap);
    idt_records[9] = create_idt_rec(interrupt_handler_9, igate_type_trap);
    idt_records[10] = create_idt_rec(interrupt_handler_10, igate_type_trap);
    idt_records[11] = create_idt_rec(interrupt_handler_11, igate_type_trap);
    idt_records[12] = create_idt_rec(interrupt_handler_12, igate_type_trap);
    idt_records[13] = create_idt_rec(interrupt_handler_13, igate_type_trap);
    idt_records[14] = create_idt_rec(interrupt_handler_14, igate_type_trap);
    idt_records[15] = create_idt_rec(interrupt_handler_15, igate_type_trap);
    idt_records[16] = create_idt_rec(interrupt_handler_16, igate_type_trap);
    idt_records[17] = create_idt_rec(interrupt_handler_17, igate_type_trap);
    idt_records[18] = create_idt_rec(interrupt_handler_18, igate_type_trap);

    asm_out(0x20, 0x11);
    asm_out(0xA0, 0x11);
    asm_out(0x21, 0x20);
    asm_out(0xA1, 0x28);
    asm_out(0x21, 0x04);
    asm_out(0xA1, 0x02);
    asm_out(0x21, 0x01);
    asm_out(0xA1, 0x01);
    asm_out(0x21, 0x0);
    asm_out(0xA1, 0x0);

    idt_records[32 + 0] = create_idt_rec(each_irq_handler_0, igate_type_trap);
    idt_records[32 + 1] = create_idt_rec(each_irq_handler_1, igate_type_trap);
    idt_records[32 + 2] = create_idt_rec(each_irq_handler_2, igate_type_trap);
    idt_records[32 + 3] = create_idt_rec(each_irq_handler_3, igate_type_trap);
    idt_records[32 + 4] = create_idt_rec(each_irq_handler_4, igate_type_trap);
    idt_records[32 + 5] = create_idt_rec(each_irq_handler_5, igate_type_trap);
    idt_records[32 + 6] = create_idt_rec(each_irq_handler_6, igate_type_trap);
    idt_records[32 + 7] = create_idt_rec(each_irq_handler_7, igate_type_trap);
    idt_records[32 + 8] = create_idt_rec(each_irq_handler_8, igate_type_trap);
    idt_records[32 + 9] = create_idt_rec(each_irq_handler_9, igate_type_trap);
    idt_records[32 + 10] = create_idt_rec(each_irq_handler_10, igate_type_trap);
    idt_records[32 + 11] = create_idt_rec(each_irq_handler_11, igate_type_trap);
    idt_records[32 + 12] = create_idt_rec(each_irq_handler_12, igate_type_trap);
    idt_records[32 + 13] = create_idt_rec(each_irq_handler_13, igate_type_trap);
    idt_records[32 + 14] = create_idt_rec(each_irq_handler_14, igate_type_trap);
    idt_records[32 + 15] = create_idt_rec(each_irq_handler_15, igate_type_trap);

    idtarray arr;
    arr.ptr = idt_records;
    arr.len = 256 * sizeof(idtrec) - 1;

    asm_lidt(arr);
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
    load_idt_recs();
    term_print(&glb_term, "salam\n");
    // init_timer(100);
    // term_clear(&glb_term);
    return 0;
}