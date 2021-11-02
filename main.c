char *frame_buffer = (char *)0xb8000;
void outb(unsigned short port, unsigned char byte);

int term_total_cols = 80;

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
    outb(0x03d4, 14);
    outb(0x03d5, index / 256);
    outb(0x03d4, 15);
    outb(0x03d5, index % 256);
}

int kmain()
{
    term_write(0, 0, 'A', color_white, color_red);
    term_set_cursor(0, 1);
    return 0;
}