#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDTARR_LEN 256
#define GDTARR_LEN 6

typedef struct
{
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes, see below
    uint16_t offset_2; // offset bits 16..31
} __attribute__((packed)) idtrec;

typedef struct
{
    unsigned short len;
    idtrec *ptr;
} __attribute__((packed)) idtarray;

typedef enum
{
    igate_type_interrupt = 0x8E,
    igate_type_trap = 0x8F,
    igate_type_task = 0x85,
} igate_type;

idtrec create_idt_rec(void *handler, igate_type type);
void load_idt_trap_recs(idtrec *idt_records);
void load_idt_hardint_recs(idtrec *idt_records);
void remap_PICs();
void load_idt_recs(idtrec *idt_records, void *int_handler, void *irq_handler);
void set_interrupt_handler(void *handler);
void set_irq_handler(void *handler);

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

#endif