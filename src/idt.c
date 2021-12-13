#include <idt.h>
#include <asm.h>
#include <util.h>

idtrec idt_records[256];
int_handler_t int_handlers[256];
int_handler_t int_handler_common;

idtrec create_idt_rec(void *handler, igate_type type)
{
    idtrec rec;
    rec.zero = 0;
    rec.type_attr = type | 0x60;
    rec.selector = 0x08;
    rec.offset_1 = (int)handler % (UINT16_MAX + 1);
    rec.offset_2 = (int)handler / (UINT16_MAX + 1);
    return rec;
}

void load_idt_trap_recs(idtrec *idt_records)
{
    idt_records[0] = create_idt_rec(interrupt_handler_0, igate_type_interrupt);
    idt_records[1] = create_idt_rec(interrupt_handler_1, igate_type_interrupt);
    idt_records[2] = create_idt_rec(interrupt_handler_2, igate_type_interrupt);
    idt_records[3] = create_idt_rec(interrupt_handler_3, igate_type_interrupt);
    idt_records[4] = create_idt_rec(interrupt_handler_4, igate_type_interrupt);
    idt_records[5] = create_idt_rec(interrupt_handler_5, igate_type_interrupt);
    idt_records[6] = create_idt_rec(interrupt_handler_6, igate_type_interrupt);
    idt_records[7] = create_idt_rec(interrupt_handler_7, igate_type_interrupt);
    idt_records[8] = create_idt_rec(interrupt_handler_8, igate_type_interrupt);
    idt_records[9] = create_idt_rec(interrupt_handler_9, igate_type_interrupt);
    idt_records[10] = create_idt_rec(interrupt_handler_10, igate_type_interrupt);
    idt_records[11] = create_idt_rec(interrupt_handler_11, igate_type_interrupt);
    idt_records[12] = create_idt_rec(interrupt_handler_12, igate_type_interrupt);
    idt_records[13] = create_idt_rec(interrupt_handler_13, igate_type_interrupt);
    idt_records[14] = create_idt_rec(interrupt_handler_14, igate_type_interrupt);
    idt_records[15] = create_idt_rec(interrupt_handler_15, igate_type_interrupt);
    idt_records[16] = create_idt_rec(interrupt_handler_16, igate_type_interrupt);
    idt_records[17] = create_idt_rec(interrupt_handler_17, igate_type_interrupt);
    idt_records[18] = create_idt_rec(interrupt_handler_18, igate_type_interrupt);

    idt_records[0x80] = create_idt_rec(interrupt_handler_128, igate_type_interrupt);
}

void load_idt_hardint_recs(idtrec *idt_records)
{
    idt_records[32 + 0] = create_idt_rec(each_irq_handler_0, igate_type_interrupt);
    idt_records[32 + 1] = create_idt_rec(each_irq_handler_1, igate_type_interrupt);
    idt_records[32 + 2] = create_idt_rec(each_irq_handler_2, igate_type_interrupt);
    idt_records[32 + 3] = create_idt_rec(each_irq_handler_3, igate_type_interrupt);
    idt_records[32 + 4] = create_idt_rec(each_irq_handler_4, igate_type_interrupt);
    idt_records[32 + 5] = create_idt_rec(each_irq_handler_5, igate_type_interrupt);
    idt_records[32 + 6] = create_idt_rec(each_irq_handler_6, igate_type_interrupt);
    idt_records[32 + 7] = create_idt_rec(each_irq_handler_7, igate_type_interrupt);
    idt_records[32 + 8] = create_idt_rec(each_irq_handler_8, igate_type_interrupt);
    idt_records[32 + 9] = create_idt_rec(each_irq_handler_9, igate_type_interrupt);
    idt_records[32 + 10] = create_idt_rec(each_irq_handler_10, igate_type_interrupt);
    idt_records[32 + 11] = create_idt_rec(each_irq_handler_11, igate_type_interrupt);
    idt_records[32 + 12] = create_idt_rec(each_irq_handler_12, igate_type_interrupt);
    idt_records[32 + 13] = create_idt_rec(each_irq_handler_13, igate_type_interrupt);
    idt_records[32 + 14] = create_idt_rec(each_irq_handler_14, igate_type_interrupt);
    idt_records[32 + 15] = create_idt_rec(each_irq_handler_15, igate_type_interrupt);
}

void remap_PICs()
{
    asm_outb(0x20, 0x11);
    asm_outb(0xA0, 0x11);
    asm_outb(0x21, 0x20);
    asm_outb(0xA1, 0x28);
    asm_outb(0x21, 0x04);
    asm_outb(0xA1, 0x02);
    asm_outb(0x21, 0x01);
    asm_outb(0xA1, 0x01);
    asm_outb(0x21, 0x0);
    asm_outb(0xA1, 0x0);
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

void load_idt_recs(int_handler_t common_handler)
{
    set_interrupt_handler(interrupt_handler);
    set_irq_handler(irq_handler);
    load_idt_trap_recs(idt_records);
    load_idt_hardint_recs(idt_records);
    remap_PICs();

    idtarray arr;
    arr.ptr = idt_records;
    arr.len = IDTARR_LEN * sizeof(idtrec) - 1;

    memset(int_handlers, 0, 256 * sizeof(int_handler_t));
    int_handler_common = common_handler;

    asm_lidt(arr);
}

void load_int_handler(uint8_t code, int_handler_t handler)
{
    int_handlers[code] = handler;
}

void interrupt_handler(registers *regs)
{
    int_handler_t handler = int_handlers[regs->int_no];
    if (!handler)
    {
        handler = int_handler_common;
    }
    handler(regs);
}