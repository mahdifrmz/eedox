#ifndef MULTSK_H
#define MULTSK_H

#include <stdint.h>
#include <paging.h>
#include <descriptor.h>
#include <pathbuf.h>

#define KERNEL_STACK_SIZE 0x2000
#define USER_STACK_SIZE 0x2000

#define TASK_WAIT_NONE 1
#define TASK_WAIT_PID 2
#define TASK_WAIT_ALL 3

extern uint32_t kernel_stack_ptr;
extern uint32_t user_stack_ptr;

typedef struct task_t task_t;

struct task_t
{
    uint32_t pid;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    page_directory_t *page_dir;
    fd_table table;
    pathbuf_t cwd;
    task_t *parent;
    int16_t exit_status;
    task_t *chwait;
    uint8_t wait;
};

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
task_t *multsk_find_zombie(task_t *task);
void multsk_wait_all(task_t *task);
task_t *multsk_gettask(uint32_t pid);
void multsk_killtask(task_t *task);
void multsk_orphan_all(task_t *task);

#endif