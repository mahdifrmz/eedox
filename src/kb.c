#include <kb.h>
#include <stdint.h>
#include <task.h>
#include <util.h>
#include <kstring.h>
#include <lock.h>
#include <terminal.h>
#include <asm.h>
#include <kutil.h> // FIXME

#define KEY_SHIFT 42
#define KEY_ALT 56
#define KEY_CTRL 29

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
kqueue_t input_list;
task_t *reader_task = NULL;

uint32_t keyboard_input_size = 0;

uint8_t key_shift = 0;
uint8_t key_ctrl = 0;
uint8_t key_alt = 0;

void setkey(uint8_t keycode)
{
    if(keycode == KEY_SHIFT)
    {
        key_shift = 1;
    }
    if(keycode == KEY_SHIFT + 128)
    {
        key_shift = 0;
    }
    if(keycode == KEY_CTRL)
    {
        key_ctrl = 1;
    }
    if(keycode == KEY_CTRL + 128)
    {
        key_ctrl = 0;
    }
    if(keycode == KEY_ALT)
    {
        key_alt = 1;
    }
    if(keycode == KEY_ALT + 128)
    {
        key_alt = 0;
    }
}

void keyboard_eof()
{
    keyboard_input_size = 0;
    kqueue_push(&input_list, (uint32_t)kstring_str(&terminal_buffer));
    terminal_buffer = kstring_new();
    if (reader_task && multitasking_flag)
    {
        task_awake(reader_task);
        reader_task = NULL;
    }
}

void keyboard_handler(__attribute__((unused)) registers *regs)
{
    uint8_t scancode = asm_inb(0x60);
    setkey(scancode);
    // kprintf("SCANCODE = %u\n",scancode);
    if (scancode < 128)
    {
        char ch = kbchars[scancode];
        if (ch != 0)
        {
            if(ch == 'd' && key_ctrl)
            {
                keyboard_eof();
            }
            else if (ch == '\b')
            {
                if (keyboard_input_size)
                {
                    keyboard_input_size--;
                    kstring_erase(&terminal_buffer, terminal_buffer.size - 1, 1);
                    term_backspace(&glb_term);
                }
            }
            else if(!key_ctrl)
            {
                if(key_shift)
                {
                    ch -= 32;
                }
                keyboard_input_size++;
                term_write_char(&glb_term, ch);
                kstring_push(&terminal_buffer, ch);
                if (ch == '\n')
                {
                    keyboard_eof();
                }
            }
        }
    }
}

void keyboard_init()
{
    terminal_buffer = kstring_new();
    input_list = kqueue_new();
    load_int_handler(INTCODE_KEYBOARD, keyboard_handler);
}