#ifndef MULTSK_H
#define MULTSK_H

#include <stdint.h>
#include <paging.h>
#include <descriptor.h>

#define KERNEL_STACK_SIZE 0x2000
#define USER_STACK_SIZE 0x2000

extern uint32_t kernel_stack_ptr;
extern uint32_t user_stack_ptr;

typedef struct
{
    uint32_t pid;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    page_directory_t *page_dir;
    uint32_t kernel_stack;
    fd_table table;

} task_t;

extern uint8_t multsk_flag;

void multsk_switch(uint32_t sleep);
uint32_t multsk_fork();
void multsk_init();
uint32_t multk_getpid();
void multsk_awake(task_t *task);
void multsk_yield();
void multsk_sleep();
task_t *multsk_curtask();
void multsk_timer(registers *regs);
void multsk_terminate();
void multsk_free(task_t *task);

#endif