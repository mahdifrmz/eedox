#include <alloc-internal.h>
#include <../src/util.h>

static const uint8_t word_size = sizeof(size_t);
static const uint8_t header_size = sizeof(object_header);

size_t alignment(size_t size, size_t align)
{
    if (!align)
    {
        return size;
    }
    else
    {
        uint8_t mod = size % align;
        size_t c = size - mod;
        if (mod != 0)
        {
            c += align;
        }
        return c;
    }
}

size_t heap_touched_memory(const heap_t *mem)
{
    return mem->brk;
}
size_t heap_total_memory(const heap_t *mem)
{
    return mem->size;
}
size_t heap_untouched_memory(const heap_t *mem)
{
    return heap_total_memory(mem) - heap_touched_memory(mem);
}

size_t heap_used_memory(const heap_t *mem)
{
    size_t sum = 0;
    object_header *beg = (object_header *)mem->root;
    while (!object_header_is_null(beg, mem))
    {
        if (object_header_get_used(beg))
        {
            sum += object_header_get_size(beg);
        }
        sum += header_size;
        beg = object_header_next(beg);
    }
    return sum;
}

size_t heap_free_memory(const heap_t *mem)
{
    return heap_total_memory(mem) - heap_used_memory(mem);
}

object_header object_header_new(size_t size)
{
    object_header o_head;
    object_header_set_size(&o_head, size);
    object_header_set_used(&o_head, 1);
    o_head.next_free = NULL;
    o_head.prev_free = NULL;
    return o_head;
}

void *heap_buffer(const heap_t *mem)
{
    return mem->root;
}

object_header *object_header_next(const object_header *o_head)
{
    size_t *next = (size_t *)o_head + object_header_get_size(o_head) / word_size + header_size / word_size;
    return (object_header *)next;
}

object_header *heap_end(const heap_t *mem)
{
    return (object_header *)(mem->root + mem->brk / word_size);
}

heap_t heap_new(void *pointer, size_t size)
{
    heap_t mem;
    mem.size = size;
    mem.brk = 0;
    mem.root = (size_t *)pointer;
    mem.free = NULL;
    return mem;
}

uint8_t object_header_get_used(const object_header *o_head)
{
    return o_head->used;
}

void object_header_set_used(object_header *o_head, uint8_t flag)
{
    o_head->used = flag;
}

size_t object_header_get_size(const object_header *o_head)
{
    return o_head->size;
}

void object_header_set_size(object_header *o_head, size_t size)
{
    o_head->size = size;
}

uint8_t object_header_is_null(const object_header *o_head, const heap_t *mem)
{
    return (size_t *)o_head >= mem->root + mem->brk / word_size;
}

object_header *heap_find_block(heap_t *mem, size_t size)
{
#if BLOCK_FINDER == SEQUENTIAL
    return NULL;
#elif BLOCK_FINDER == FIRST_FIT
    return heap_find_first_fit(mem, size);
#elif BLOCK_FINDER == BEST_FIT
    return heap_find_best_fit(mem, size);
#else
#error invalid block finder id
#endif
}

object_header *heap_find_first_fit(heap_t *mem, size_t size)
{
    object_header *block = (object_header *)mem->free;
    while (block != NULL)
    {
        if (object_header_get_size(block) >= size)
        {
            return block;
        }
        else
        {
            block = block->next_free;
        }
    }
    return NULL;
}

void object_header_merge(object_header *o_head)
{
    object_header *next = object_header_next(o_head);
    size_t new_size = object_header_get_size(o_head) + object_header_get_size(next) + header_size;
    object_header_set_size(o_head, new_size);
}

object_header *object_header_try_split(object_header *o_head, size_t size, heap_t *mem)
{
    size_t current_size = object_header_get_size(o_head);
    if (heap_is_end(mem, o_head))
    {
        if (size < current_size)
        {
            size_t rem_size = current_size - size;
            mem->brk -= rem_size;
            object_header_set_size(o_head, size);
        }
    }
    else
    {
        if (size + header_size < current_size)
        {
            size_t rem_size = current_size - size - header_size;
            object_header_set_size(o_head, size);
            object_header *next = object_header_next(o_head);
            *next = object_header_new(rem_size);
            object_header_set_used(next, 0);
            heap_add_free(mem, next);
            return next;
        }
    }
    return NULL;
}

object_header *heap_find_best_fit(heap_t *mem, size_t size)
{
    object_header *best = NULL;
    object_header *block = (object_header *)mem->free;
    while (block != NULL)
    {
        size_t block_size = object_header_get_size(block);
        if (block_size >= size)
        {
            if (block_size == size)
            {
                return block;
            }
            else if (best == NULL || block_size < object_header_get_size(best))
            {
                best = block;
            }
            block = block->next_free;
        }
        else
        {
            if (heap_is_end(mem, block))
            {
                size_t needed_size = size - block_size;
                if (heap_untouched_memory(mem) >= needed_size)
                {
                    mem->brk += needed_size;
                    object_header_set_size(block, size);
                    return block;
                }
                else
                {
                    block = block->next_free;
                }
            }
            else
            {
                object_header *next = object_header_next(block);
                if (!object_header_is_null(next, mem) && !object_header_get_used(next))
                {
                    object_header_merge(block);
                    heap_remove_free(mem, next);
                }
                else
                {
                    block = block->next_free;
                }
            }
        }
    }
    return best;
}

uint8_t heap_is_end(const heap_t *mem, const object_header *header)
{
    return (size_t *)(header + 1) + object_header_get_size(header) / word_size == (size_t *)heap_end(mem);
}

void *heap_alloc(heap_t *mem, size_t size, size_t align)
{
    if (size == 0)
    {
        return NULL;
    }
    size = alignment(size, align);
    object_header *block = heap_find_block(mem, size);
    if (block != NULL)
    {
        object_header_try_split(block, size, mem);
        object_header_set_used(block, 1);
        heap_remove_free(mem, block);
        return object_header_get_pointer(block);
    }
    else if (size + header_size > heap_untouched_memory(mem))
    {
        return NULL;
    }
    object_header *end = heap_end(mem);
    void *pointer = object_header_get_pointer(end);
    *end = object_header_new(size);
    mem->brk += header_size + size;
    return pointer;
}

void *heap_realloc(heap_t *mem, void *pointer, size_t size, size_t align)
{
    if (size == 0)
    {
        heap_dealloc(mem, pointer);
        return NULL;
    }
    size = alignment(size, align);
    object_header *header = object_header_from_pointer(pointer);
    size_t current_size = object_header_get_size(header);
    if (size == current_size)
    {
        return pointer;
    }
    else if (size < current_size)
    {
        object_header_try_split(header, size, mem);
        return pointer;
    }
    // ..........
    while (1)
    {
        size_t current_size = object_header_get_size(header);
        object_header *next = object_header_next(header);
        if (current_size >= size)
        {
            return heap_realloc(mem, pointer, size, align);
        }
        if (object_header_get_used(next) == 0)
        {
            object_header_merge(header);
        }
        else
        {
            break;
        }
    }
    void *new_block = heap_alloc(mem, size, align);
    if (new_block == NULL)
    {
        return NULL;
    }
    memcpy(pointer, new_block, current_size);
    heap_dealloc(mem, pointer);
    return new_block;
}

object_header *object_header_from_pointer(void *pointer)
{
    return (object_header *)pointer - 1;
}

void *object_header_get_pointer(object_header *o_head)
{
    return o_head + 1;
}

void heap_dealloc(heap_t *mem, void *pointer)
{
    object_header *header = object_header_from_pointer(pointer);
    object_header_set_used(header, 0);
    heap_add_free(mem, header);
}

void heap_add_free(heap_t *mem, object_header *header)
{
    header->next_free = mem->free;
    header->prev_free = NULL;
    if (mem->free != NULL)
    {
        ((object_header *)mem->free)->prev_free = header;
    }
    mem->free = header;
}

void heap_remove_free(heap_t *mem, object_header *header)
{
    object_header *next = header->next_free;
    object_header *prev = header->prev_free;
    if (next != NULL)
    {
        next->prev_free = prev;
    }
    if (prev != NULL)
    {
        prev->next_free = next;
    }
    header->prev_free = NULL;
    header->next_free = NULL;
    if (mem->free == header)
    {
        mem->free = next;
    }
}

void heap_expand(heap_t *mem,size_t size)
{
    mem->size += size;
}