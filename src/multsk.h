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
    uint32_t kernel_stack;
} task_t;

extern uint8_t multsk_flag;

void multsk_switch();
uint32_t multsk_fork();
void multsk_init();
uint32_t multk_getpid();
void multsk_awake(task_t *task);
void multsk_yield();
void multsk_sleep();
task_t *multsk_curtask();

#endif