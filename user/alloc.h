#include <stdint.h>

#ifdef __i386__
typedef uint32_t size_t;
#else
typedef uint64_t size_t;
#endif
#define NULL (void *)0x0

struct heap_t
{
    size_t *root;
    size_t size;
    size_t brk;
    void *free;
};
typedef struct heap_t heap_t;

size_t heap_touched_memory(const heap_t *mem);
size_t heap_total_memory(const heap_t *mem);
size_t heap_untouched_memory(const heap_t *mem);
size_t heap_used_memory(const heap_t *mem);
size_t heap_free_memory(const heap_t *mem);
void *heap_buffer(const heap_t *mem);
heap_t heap_new(void *pointer, size_t size);
void *heap_alloc(heap_t *mem, size_t size, size_t align);
void *heap_realloc(heap_t *mem, void *pointer, size_t size, size_t align);
void heap_dealloc(heap_t *mem, void *pointer);
void heap_expand(heap_t *mem,size_t size);