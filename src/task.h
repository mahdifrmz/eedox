#ifndef task_H
#define task_H

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
    uint32_t brk;
    page_directory_t *page_dir;
    fd_table table;
    pathbuf_t cwd;
    task_t *parent;
    int16_t exit_status;
    task_t *chwait;
    uint8_t wait;
};

extern uint8_t multitasking_flag;

void task_switch(uint32_t sleep);
uint32_t task_fork();
void multitasking_init();
uint32_t multk_getpid();
void task_awake(task_t *task);
void task_yield();
void task_sleep();
task_t *task_curtask();
void task_timer(registers *regs);
void task_free(task_t *task);
task_t *task_find_zombie(task_t *task);
void task_wait_all(task_t *task);
task_t *task_gettask(uint32_t pid);
void task_killtask(task_t *task);
void task_orphan_all(task_t *task);
void task_close_fd(uint32_t fd_id);
void task_close_all_fds();

#endif