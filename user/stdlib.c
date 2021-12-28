#include <stdlib.h>

int32_t _printf(const char *message, va_list args)
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
        write(STDOUT, local, strlen(local));
    }
    return count;
}

int32_t printf(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    uint32_t count = _printf(message, args);
    va_end(args);
    return count;
}
