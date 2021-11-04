#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define TERMCOLS 80
#define TERMROWS 25

typedef enum
{
    color_white = 15,
    color_black = 0,
    color_red = 4,
    color_blue = 1,
} color;

typedef struct
{
    int active;
    int currow;
    int curcol;
    char cells[TERMROWS][TERMCOLS];
} terminal_t;

void term_scrollup(terminal_t *term);
void term_write_char(terminal_t *term, char c);
void term_init(terminal_t *term);
void term_clear(terminal_t *term);
void term_print_buffer(terminal_t *term, char *str, int len);
void term_print(terminal_t *term, char *str);
void term_print_dword_dec(terminal_t *term, uint32_t i);
void term_print_flag(terminal_t *term);
void term_print_endl(terminal_t *term);

void term_raw_set_cursor(int row, int col);
void term_raw_putchar(int row, int col, char c, color fg, color bg);

#endif