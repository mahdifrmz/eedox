#ifndef MULTSK_H
#define MULTSK_H

#include <stdint.h>
#include <paging.h>

typedef struct
{
    uint32_t pid;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    page_directory_t *page_dir;
} task_t;

extern uint8_t multsk_flag;

void multsk_switch();
void multsk_fork();
void multsk_init();
uint32_t multk_getpid();

#endif