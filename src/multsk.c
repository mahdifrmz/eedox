#include <multsk.h>
#include <kqueue.h>
#include <kutil.h>
#include <asm.h>
#include <fs.h>

#define KERNEL_STACK_SIZE 0x2000
#define INIT_PID 1

kqueue_t rr_queue;
uint32_t eip_buffer, esp_buffer, ebp_buffer;
extern page_directory_t *current_page_directory;
extern tss_rec tss_entry;
uint8_t multsk_flag = 0;
uint32_t task_count = 0;
task_t *current_task;
uint32_t kernel_stack_ptr;
uint32_t user_stack_ptr;
vec_t tasklist; // keeps task_t* pointers

uint32_t multk_getpid()
{
    return current_task->pid;
}

void multsk_switch(uint32_t sleep)
{
    if (rr_queue.size == 1)
    {
        return;
    }
    task_t *curtask = (task_t *)kqueue_pop(&rr_queue);
    if (sleep == 2) // termination
    {
        multsk_free(curtask);
    }
    else
    {
        curtask->ebp = asm_get_ebp();
        curtask->esp = asm_get_esp();
        curtask->eip = asm_get_eip();

        if (curtask->eip == 0xffffffff)
        {
            return;
        }
    }
    if (sleep == 0) // preemption
    {
        kqueue_push(&rr_queue, (uint32_t)curtask);
    }
    task_t *nextask = (task_t *)kqueue_peek(&rr_queue);
    eip_buffer = nextask->eip;
    ebp_buffer = nextask->ebp;
    esp_buffer = nextask->esp;
    current_task = nextask;
    current_page_directory = current_task->page_dir;
    asm_multsk_switch();
}

void multsk_terminate()
{
    multsk_switch(2);
}

uint32_t multsk_fork()
{
    task_t *curtask = (task_t *)kqueue_peek(&rr_queue);
    task_t *newtask = kmalloc(sizeof(task_t));
    newtask->pid = task_count++;
    newtask->ebp = asm_get_ebp();
    newtask->esp = asm_get_esp();
    newtask->eip = asm_get_eip();
    if (newtask->eip != 0xffffffff)
    {
        newtask->exit_status = -1;
        newtask->wait = TASK_WAIT_NONE;
        newtask->page_dir = page_directory_clone(curtask->page_dir);
        newtask->table = fd_table_clone(&curtask->table);
        kqueue_push(&rr_queue, (uint32_t)newtask);
        newtask->cwd = pathbuf_copy(&curtask->cwd);
        newtask->parent = curtask;
        newtask->chwait = NULL;
        vec_push(&tasklist, (uint32_t)newtask);
        return newtask->pid;
    }
    else
    {
        return 0;
    }
}

void multsk_awake(task_t *task)
{
    kqueue_push(&rr_queue, (uint32_t)task);
}

task_t *multsk_curtask()
{
    return (task_t *)kqueue_peek(&rr_queue);
}

fd_table init_fdt()
{
    fd_table table = fd_table_create(2);

    fd_t stdin;
    stdin.access = FD_ACCESS_READ;
    stdin.ptr = NULL;
    stdin.kind = FD_KIND_STDIN;
    stdin.isopen = 1;

    fd_t stdout;
    stdout.access = FD_ACCESS_WRITE;
    stdout.ptr = NULL;
    stdout.kind = FD_KIND_STDOUT;
    stdout.isopen = 1;

    fd_table_add(&table, stdin);
    fd_table_add(&table, stdout);

    return table;
}

void multsk_free(task_t *task)
{
    pathbuf_free(&task->cwd);
    kfree(task->table.records);
    kfree(task->page_dir);
}

task_t *multsk_gettask(uint32_t pid)
{
    return (task_t *)tasklist.buffer[pid];
}

void multsk_killtask(task_t *task)
{
    uint32_t pid = task->pid;
    multsk_orphan_all(task);
    multsk_free(task);
    tasklist.buffer[pid] = 0;
}

task_t *multsk_find_zombie(task_t *task)
{
    for (uint32_t i = 0; i < tasklist.size; i++)
    {
        task_t *child = (task_t *)tasklist.buffer[i];
        if (child->parent == task && child->exit_status != -1)
        {
            return child;
        }
    }
    return NULL;
}

void multsk_orphan_all(task_t *task)
{
    for (uint32_t i = 0; i < tasklist.size; i++)
    {
        task_t *child = (task_t *)tasklist.buffer[i];
        if (child->parent == task)
        {
            child->parent = (task_t *)tasklist.buffer[INIT_PID];
        }
    }
}

void multsk_init()
{
    task_t *first = kmalloc(sizeof(task_t));
    tasklist = vec_new();
    first->pid = task_count++;
    first->page_dir = current_page_directory;
    rr_queue = kqueue_new();
    kqueue_push(&rr_queue, (uint32_t)first);
    current_task = first;
    tss_entry.esp0 = kernel_stack_ptr + KERNEL_STACK_SIZE;
    multsk_flag = 1;
    first->table = init_fdt();
    first->cwd = pathbuf_root();
    first->wait = TASK_WAIT_NONE;
    first->exit_status = -1;
    first->chwait = NULL;
    first->parent = NULL;
    vec_push(&tasklist, (uint32_t)first);
    load_int_handler(INTCODE_PIC, multsk_timer);
}

void multsk_close_fd(uint32_t fd_id)
{
    task_t *task = multsk_curtask();
    if (fd_id >= task->table.size)
    {
        return;
    }
    fd_t *fd = &task->table.records[fd_id];
    if (!fd->isopen)
    {
        return;
    }
    fd_table_rem(&task->table, fd_id);
    if (fd->kind == FD_KIND_DISK || fd->kind == FD_KIND_DIR)
    {
        fs_close(fd->ptr);
    }
}

void multsk_close_all_fds()
{
    task_t* task = multsk_curtask();
    for(uint32_t i=0;i<task->table.size;i++)
    {
        multsk_close_fd(i);
    }
}

void multsk_timer(__attribute__((unused)) registers *regs)
{
    multsk_switch(0);
}