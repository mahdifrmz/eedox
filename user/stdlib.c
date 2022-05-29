#include <stdlib.h>

int _printf(const char *message, va_list args)
{
    int count = 0;
    char local[PARTIAL_PRINT_BUFFER_SIZE + 16];
    int index = 0;
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

int printf(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    uint32_t count = _printf(message, args);
    va_end(args);
    return count;
}

int fmain(int argc, const char** argv);

void startup(int argc,const char** argv)
{
    int status_code = fmain(argc,argv);
    exit((short)status_code);
}