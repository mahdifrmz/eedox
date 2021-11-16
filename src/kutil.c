#include <kutil.h>
#include <stdarg.h>
#include <kstring.h>

extern terminal_t glb_term;
extern heap_t glb_heap;

void *kmalloc(uint32_t size)
{
    return heap_alloc(&glb_heap, size, 0);
}

void *kmalloc_a(uint32_t size)
{
    return heap_alloc(&glb_heap, size, 1);
}

void kfree(void *ptr)
{
    heap_free(&glb_heap, ptr);
}

void kpanic(const char *message)
{
    kprintf("PANIC! ( %s )", message);
    while (1)
    {
    }
}

int32_t kprintf(const char *message, ...)
{
    va_list args;
    int32_t count = 0;
    va_start(args, message);
    char local[PARTIAL_PRINT_BUFFER_SIZE + 16];
    int32_t index = 0;
    while ((uint32_t)index < strlen(message))
    {
        memset(local, 0, sizeof(local));
        index = partial_print(local, index, message, args);
        if (index == -1)
        {
            return -1;
        }
        count += strlen(local);
        term_print(&glb_term, local);
    }
    va_end(args);
    return count;
}