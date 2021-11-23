#include <multsk.h>
#include <kqueue.h>
#include <kutil.h>
#include <asm.h>

#define KERNEL_STACK_SIZE 0x2000

kqueue_t rr_queue;
uint32_t eip_buffer, esp_buffer, ebp_buffer;
extern page_directory_t *current_page_directory;
extern tss_rec tss_entry;
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
        tss_entry.esp0 = curtask->kernel_stack + KERNEL_STACK_SIZE;
        asm_multsk_switch();
    }
}

uint32_t multsk_fork()
{
    asm_cli();
    task_t *curtask = (task_t *)kqueue_peek(&rr_queue);
    task_t *newtask = kmalloc(sizeof(task_t));
    newtask->kernel_stack = (uint32_t)kmalloc_a(KERNEL_STACK_SIZE);
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

void switch_to_user_mode()
{
    asm volatile("  \
        cli; \
        mov $0x23, %ax; \
        mov %ax, %ds; \
        mov %ax, %es; \
        mov %ax, %fs; \
        mov %ax, %gs; \
                      \
        mov %esp, %eax; \
        pushl $0x23; \
        pushl %eax; \
        pushf; \
        pushl $0x1B; \
        push $1f; \
        iret; \
   1: \
    ");
}

void multsk_init()
{
    task_t *first = kmalloc(sizeof(task_t));
    first->pid = task_count++;
    first->page_dir = current_page_directory;
    first->kernel_stack = (uint32_t)kmalloc_a(KERNEL_STACK_SIZE);
    rr_queue = kqueue_new();
    kqueue_push(&rr_queue, (uint32_t)first);
    multsk_flag = 1;
}

void multsk_set_kernel_stack(uint32_t esp)
{
    tss_entry.esp0 = esp;
}