#include <stdint.h>

char *frame_buffer = (char *)0xb8000;
#define TERMCOLS 80
#define TERMROWS 25

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
    unsigned short length;
    unsigned short base_l;
    unsigned char base_m;
    unsigned char flags;
    unsigned char fllen;
    unsigned char base_h;
} __attribute__((packed)) gdtrec;

typedef struct
{
    unsigned short len;
    gdtrec *ptr;
} __attribute__((packed)) gdtarray;

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

typedef enum
{
    color_white = 15,
    color_black = 0,
    color_red = 4,
    color_blue = 1,
} color;

void term_print_buffer(char *str, int len);
void term_print(char *str);
void term_print_dword_dec(uint32_t i);

void interrupt_handler(registers regs)
{
    if (regs.int_no != 32)
    {
        term_print("interrupt ");
        term_print_dword_dec(regs.int_no);
        term_print("\n");
    }
}

void asm_lgdt(gdtarray);
void asm_lidt(idtarray);
void asm_load_segregs();
void asm_out(unsigned short port, unsigned char byte);

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

gdtrec gdt_records[3];
idtrec idt_records[256];

void load_gdt_recs()
{
    // null : 0x00
    gdt_records[0].length = 0;
    gdt_records[0].base_l = 0;
    gdt_records[0].base_m = 0;
    gdt_records[0].flags = 0;
    gdt_records[0].fllen = 0;
    gdt_records[0].base_h = 0;

    // code : 0x08
    gdt_records[1].length = 0xffff;
    gdt_records[1].base_l = 0;
    gdt_records[1].base_m = 0;
    gdt_records[1].flags = 0b10011010;
    gdt_records[1].fllen = 0b11001111;
    gdt_records[1].base_h = 0;

    // data : 0x10
    gdt_records[2].length = 0xffff;
    gdt_records[2].base_l = 0;
    gdt_records[2].base_m = 0;
    gdt_records[2].flags = 0b10010010;
    gdt_records[2].fllen = 0b11001111;
    gdt_records[2].base_h = 0;

    gdtarray arr;
    arr.ptr = gdt_records;
    arr.len = sizeof(gdt_records) - 1;

    asm_lgdt(arr);
    asm_load_segregs();
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
    arr.len = sizeof(idt_records) - 1;

    asm_lidt(arr);
}

void init_timer(uint32_t frequency)
{
    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint16_t divisor = 1193180 / frequency;

    // Send the command byte.
    asm_out(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor % 256);
    uint8_t h = (uint8_t)(divisor / 256);

    // Send the frequency divisor.
    asm_out(0x40, l);
    asm_out(0x40, h);
}

void term_putchar(int row, int col, char c, color fg, color bg)
{
    int index = row * TERMCOLS + col;
    frame_buffer[index * 2] = c;
    frame_buffer[index * 2 + 1] = bg * 16 + fg;
}

void term_set_cursor(int row, int col)
{
    unsigned short index = row * TERMCOLS + col;
    asm_out(0x03d4, 14);
    asm_out(0x03d5, index / 256);
    asm_out(0x03d4, 15);
    asm_out(0x03d5, index % 256);
}

int term_currow = 0;
int term_curcol = 0;
char terminal[TERMROWS][TERMCOLS];

void term_scrollup()
{
    for (int i = 0; i < TERMROWS - 1; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            char c = terminal[i + 1][j];
            terminal[i][j] = c;
            term_putchar(i, j, c, color_white, color_black);
        }
    }
    for (int j = 0; j < TERMCOLS; j++)
    {
        terminal[TERMROWS - 1][j] = '\0';
        term_putchar(TERMROWS - 1, j, '\0', color_white, color_black);
    }
}

void term_write_char(char c)
{
    if (c == '\n')
    {
        term_curcol = 0;
        term_currow++;
    }
    else
    {
        term_putchar(term_currow, term_curcol, c, color_white, color_black);
        terminal[term_currow][term_curcol] = c;
        term_curcol++;
        if (term_curcol == TERMCOLS)
        {
            term_curcol = 0;
            term_currow++;
        }
    }
    if (term_currow == TERMROWS)
    {
        term_scrollup();
        term_currow--;
    }
    term_set_cursor(term_currow, term_curcol);
}

void term_init()
{
    for (int i = 0; i < TERMROWS; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            terminal[i][j] = '\0';
        }
    }
    term_curcol = 0;
    term_currow = 0;
    term_set_cursor(0, 0);
}

void term_clear()
{
    for (int i = 0; i < TERMROWS; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            terminal[i][j] = '\0';
            term_putchar(i, j, '\0', color_white, color_black);
        }
    }
    term_curcol = 0;
    term_currow = 0;
    term_set_cursor(0, 0);
}

void term_print_buffer(char *str, int len)
{
    for (int i = 0; i < len; i++)
    {
        term_write_char(str[i]);
    }
}

void term_print(char *str)
{
    while (*str != '\0')
    {
        term_write_char(*(str++));
    }
}

void term_print_dword_dec(uint32_t i)
{
    if (!i)
    {
        term_print("0");
        return;
    }
    const int STACK_LEN = 11;
    char stack[STACK_LEN];
    stack[STACK_LEN - 1] = '\0';
    int sidx = STACK_LEN - 2;
    while (i != 0)
    {
        stack[sidx--] = (i % 10) + '0';
        i /= 10;
    }
    term_print(stack + sidx + sizeof(char));
}

void term_print_flag()
{
    term_print("flag\n");
}

void term_print_endl()
{
    term_print("\n");
}

int kmain()
{
    term_init();
    load_gdt_recs();
    load_idt_recs();
    init_timer(1);
    // term_clear();
    return 0;
}