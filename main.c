char *frame_buffer = (char *)0xb8000;
int term_total_cols = 80;

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

void asm_lgdt(gdtarray);
void asm_load_segregs(int code_seg, int data_seg);
void asm_out(unsigned short port, unsigned char byte);

void load_gdt_recs()
{
    gdtrec gdt_records[3];
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
    arr.len = sizeof(gdtrec) * 3 - 1;

    asm_lgdt(arr);
    asm_load_segregs(1 * sizeof(gdtrec), 2 * sizeof(gdtrec));
}

typedef enum
{
    color_white = 15,
    color_blue = 1,
    color_red = 4,
    color_black = 0
} color;

void term_write(int row, int col, char c, color fg, color bg)
{
    int index = row * term_total_cols + col;
    frame_buffer[index * 2] = c;
    frame_buffer[index * 2 + 1] = fg * 16 + bg;
}

void term_set_cursor(int row, int col)
{
    unsigned short index = row * term_total_cols + col;
    asm_out(0x03d4, 14);
    asm_out(0x03d5, index / 256);
    asm_out(0x03d4, 15);
    asm_out(0x03d5, index % 256);
}

int kmain()
{
    load_gdt_recs();
    term_write(0, 0, 'A', color_white, color_red);
    term_set_cursor(0, 1);
    return 0;
}