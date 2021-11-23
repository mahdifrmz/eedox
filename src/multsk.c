#include <multsk.h>
#include <kqueue.h>
#include <kutil.h>
#include <asm.h>

kqueue_t rr_queue;
uint32_t eip_buffer, esp_buffer, ebp_buffer;
extern page_directory_t *current_page_directory;
uint8_t multsk_flag = 0;
uint32_t task_count = 0;

uint32_t multk_getpid()
{
    return ((task_t *)rr_queue.head->value)->pid;
}

void multsk_switch()
{
    if (rr_queue.size == 1)
    {
        return;
    }
    task_t *curtask = (task_t *)kqueue_pop(&rr_queue);
    curtask->ebp = asm_get_ebp();
    curtask->esp = asm_get_esp();
    curtask->eip = asm_get_eip();
    if (curtask->eip != 0xffffffff)
    {
        kqueue_push(&rr_queue, (uint32_t)curtask);
        task_t *nextask = (task_t *)kqueue_peek(&rr_queue);
        eip_buffer = nextask->eip;
        ebp_buffer = nextask->ebp;
        esp_buffer = nextask->esp;
        current_page_directory = nextask->page_dir;
        asm_multsk_switch();
    }
}

uint32_t multsk_fork()
{
    asm_cli();
    task_t *curtask = (task_t *)kqueue_peek(&rr_queue);
    task_t *newtask = kmalloc(sizeof(task_t));
    newtask->pid = task_count++;
    newtask->ebp = asm_get_ebp();
    newtask->esp = asm_get_esp();
    newtask->eip = asm_get_eip();
    if (newtask->eip != 0xffffffff)
    {
        newtask->page_dir = page_directory_clone(curtask->page_dir);
        kqueue_push(&rr_queue, (uint32_t)newtask);
    }
    uint32_t pid = multk_getpid();
    asm_sti();
    return pid;
}

void multsk_init()
{
    task_t *first = kmalloc(sizeof(task_t));
    first->pid = task_count++;
    first->page_dir = current_page_directory;
    rr_queue = kqueue_new();
    kqueue_push(&rr_queue, (uint32_t)first);
    multsk_flag = 1;
}