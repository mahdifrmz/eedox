#include <kutil.h>
#include <stdarg.h>
#include <kstring.h>

extern terminal_t glb_term;
extern heap_t glb_heap;

void kpanic(const char *message)
{
    term_print(&glb_term, "\nPANIC!(");
    if (message)
    {
        term_print(&glb_term, message);
    }
    term_print(&glb_term, ")\n");
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