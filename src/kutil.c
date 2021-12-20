#include <kutil.h>
#include <stdarg.h>
#include <kstring.h>

extern terminal_t glb_term;
extern heap_t kernel_heap;

void *kmalloc(uint32_t size)
{
    return heap_alloc(&kernel_heap, size, 0);
}

void *kmalloc_a(uint32_t size)
{
    return heap_alloc(&kernel_heap, size, 1);
}

void kfree(void *ptr)
{
    heap_free(&kernel_heap, ptr);
}

int32_t _kprintf(const char *message, va_list args)
{
    int32_t count = 0;
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
    return count;
}

char *strdup(const char *str)
{
    uint32_t len = strlen(str);
    char *dup = kmalloc(strlen(str) + 1);
    dup[len] = 0;
    strcpy(dup, str);
    return dup;
}

int32_t kprintf(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    uint32_t count = _kprintf(message, args);
    va_end(args);
    return count;
}

void kpanic(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    kprintf("PANIC! ( ");
    _kprintf(message, args);
    kprintf(" )");
    va_end(args);
    while (1)
    {
    }
}