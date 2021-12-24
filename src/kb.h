#ifndef KB_H
#define KB_H

#include <idt.h>
#include <kstring.h>
#include <lock.h>
#include <multsk.h>
#include <terminal.h>

void keyboard_handler(registers *regs);
void keyboard_init();

extern terminal_t glb_term;
extern task_t *reader_task;
extern kqueue_t input_list;

#endif