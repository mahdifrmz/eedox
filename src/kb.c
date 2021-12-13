#include <kb.h>
#include <stdint.h>
#include <multsk.h>
#include <util.h>
#include <kstring.h>
#include <lock.h>
#include <terminal.h>
#include <asm.h>
#include <kutil.h> // FIXME

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

kstring_t terminal_buffer;
kstring_t input_buffer;
krwlock reader_lock;
task_t *reader_task = NULL;

void keyboard_handler(__attribute__((unused)) registers *regs)
{
    uint8_t scancode = asm_inb(0x60);
    if (scancode < 128)
    {
        char ch = kbchars[scancode];
        if (ch != 0)
        {
            if (ch == '\b')
            {
                if (terminal_buffer.size)
                {
                    kstring_erase(&terminal_buffer, terminal_buffer.size - 1, 1);
                    term_backspace(&glb_term);
                }
            }
            else
            {
                term_write_char(&glb_term, ch);
                kstring_push(&terminal_buffer, ch);
                if (ch == '\n')
                {
                    kstring_insert(&input_buffer, input_buffer.size, kstring_str(&terminal_buffer));
                    kstring_clear(&terminal_buffer);
                    terminal_buffer.size = 0;
                    if (reader_task && multsk_flag)
                    {
                        multsk_awake(reader_task);
                        reader_task = NULL;
                    }
                }
            }
        }
    }
}

void keyboard_init()
{
    krwlock_init(&reader_lock);
    terminal_buffer = kstring_new();
    input_buffer = kstring_new();
    load_int_handler(INTCODE_KEYBOARD, keyboard_handler);
}