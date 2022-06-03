#include <stdlib.h>

heap_t heap;
int fmain(int argc, const char** argv);
extern char end;
uint32_t heap_size;

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

uint32_t heap_beg()
{
    uint32_t end_ptr = (uint32_t)&end;
    if(end_ptr % 0x1000)
    {
        end_ptr &= 0xfffff000;
        end_ptr += 0x1000;
    }
    return end_ptr;
}

void startup(int argc,const char** argv)
{
    heap_size = 1024;
    sbrk(heap_size);
    heap = heap_new((void*)heap_beg(),heap_size);
    int status_code = fmain(argc,argv);
    exit((short)status_code);
}

void* malloc(int size)
{
    void* ptr = heap_alloc(&heap,size,4);
    while(size && !ptr)
    {
        sbrk(heap_size);
        heap_expand(&heap,heap_size);
        heap_size *= 2;
        ptr = heap_alloc(&heap,size,4);
    }
    return ptr;
}

void free(void* ptr)
{
    heap_dealloc(&heap,ptr);
}

void* realloc(void* ptr,int size)
{
    void* new_ptr = heap_realloc(&heap,ptr,size,4);
    while(size && !new_ptr)
    {
        sbrk(heap_size);
        heap_size *= 2;
        new_ptr = heap_realloc(&heap,ptr,size,4);
    }
    return new_ptr;
}