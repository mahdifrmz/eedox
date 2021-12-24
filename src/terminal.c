#include <terminal.h>
#include <asm.h>

char *frame_buffer = (char *)0xb8000;

void term_raw_putchar(int row, int col, char c, color fg, color bg)
{

    int index = row * TERMCOLS + col;
    frame_buffer[index * 2] = c;
    frame_buffer[index * 2 + 1] = bg * 16 + fg;
}

void term_raw_set_cursor(int row, int col)
{
    unsigned short index = row * TERMCOLS + col;
    asm_outb(0x03d4, 14);
    asm_outb(0x03d5, index / 256);
    asm_outb(0x03d4, 15);
    asm_outb(0x03d5, index % 256);
}

void term_putchar(terminal_t *term, int row, int col, char c, color fg, color bg)
{
    if (term->active)
    {
        term_raw_putchar(row, col, c, fg, bg);
    }
}

void term_set_cursor(terminal_t *term, int row, int col)
{
    if (term->active)
    {
        term_raw_set_cursor(row, col);
    }
}

void term_scrollup(terminal_t *term)
{
    for (int i = 0; i < TERMROWS - 1; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            char c = term->cells[i + 1][j];
            term->cells[i][j] = c;
            term_putchar(term, i, j, c, color_white, color_black);
        }
    }
    for (int j = 0; j < TERMCOLS; j++)
    {
        term->cells[TERMROWS - 1][j] = '\0';
        term_putchar(term, TERMROWS - 1, j, '\0', color_white, color_black);
    }
}

void term_write_char(terminal_t *term, char c)
{
    if (c == 0)
    {
        return;
    }
    if (c == '\n')
    {
        term->curcol = 0;
        term->currow++;
    }
    else
    {
        if (term->active)
        {
            term_putchar(term, term->currow, term->curcol, c, color_white, color_black);
        }
        term->cells[term->currow][term->curcol] = c;
        term->curcol++;
        if (term->curcol == TERMCOLS)
        {
            term->curcol = 0;
            term->currow++;
        }
    }
    if (term->currow == TERMROWS)
    {
        if (term->active)
        {
            term_scrollup(term);
        }
        term->currow--;
    }
    if (term->active)
    {
        term_set_cursor(term, term->currow, term->curcol);
    }
}

char term_curc(terminal_t *term)
{
    return term->cells[term->currow][term->curcol];
}

void term_backspace(terminal_t *term)
{
    if (term->curcol == 0 && term->currow == 0)
    {
        return;
    }
    if (term->curcol == 0)
    {
        term->curcol = TERMCOLS - 1;
        term->currow--;
        while (term_curc(term) == '\0' && term->curcol >= 0)
        {
            term->curcol--;
        }
        if (term->curcol != TERMCOLS - 1)
        {
            term->curcol++;
        }
        else
        {
            term_putchar(term, term->currow, term->curcol, '\0', color_white, color_black);
        }
    }
    else
    {
        term->curcol--;
        term->cells[term->currow][term->curcol] = 0;
        term_putchar(term, term->currow, term->curcol, '\0', color_white, color_black);
    }
    term_set_cursor(term, term->currow, term->curcol);
}

void term_init(terminal_t *term)
{
    for (int i = 0; i < TERMROWS; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            term->cells[i][j] = '\0';
        }
    }
    term->active = 0;
    term->curcol = 0;
    term->currow = 0;
}

void term_fg(terminal_t *term)
{
    term->active = 1;
    term_raw_set_cursor(term->currow, term->curcol);
    for (int i = 0; i < TERMROWS; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            term_raw_putchar(i, j, term->cells[i][j], color_white, color_black);
        }
    }
}

void term_bg(terminal_t *term)
{
    term->active = 0;
}

void term_clear(terminal_t *term)
{
    for (int i = 0; i < TERMROWS; i++)
    {
        for (int j = 0; j < TERMCOLS; j++)
        {
            term->cells[i][j] = '\0';
            if (term->active)
            {
                term_putchar(term, i, j, '\0', color_white, color_black);
            }
        }
    }
    term->curcol = 0;
    term->currow = 0;
    term_set_cursor(term, 0, 0);
}

void term_print_buffer(terminal_t *term, const char *str, int len)
{
    for (int i = 0; i < len; i++)
    {
        term_write_char(term, str[i]);
    }
}

void term_print(terminal_t *term, const char *str)
{
    while (*str != '\0')
    {
        term_write_char(term, *(str++));
    }
}

void term_print_endl(terminal_t *term)
{
    term_print(term, "\n");
}
