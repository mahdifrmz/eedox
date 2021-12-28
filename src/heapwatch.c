#include <heapwatch.h>
#include <kutil.h>

void heapwatch_init(heapwatch_t *watch)
{
    watch->index = 0;
}
void heapwatch_alloc(heapwatch_t *watch, uint32_t ptr, uint32_t size)
{
    if (!size || !ptr)
    {
        return;
    }
    uint32_t free_index = 0xffffffff;
    for (uint32_t i = 0; i < watch->index; i++)
    {
        watchrec_t rec = watch->recs[i];
        if (rec.ptr == 0)
        {
            free_index = i;
            continue;
        }
        uint32_t start = ptr;
        uint32_t end = ptr + size - 1;
        uint32_t rec_start = rec.ptr;
        uint32_t rec_end = rec.ptr + rec.size - 1;
        if ((start >= rec_start && start <= rec_end) || (end >= rec_start && end <= rec_end))
        {
            kpanic("ALLOC FAULT");
        }
    }
    watchrec_t *rec;
    if (free_index != 0xffffffff)
    {
        rec = &watch->recs[free_index];
    }
    else
    {
        if (watch->index == 1024)
        {
            kpanic("WATCHER OVERFLOWED\n");
        }
        rec = &watch->recs[watch->index++];
    }
    rec->ptr = ptr;
    rec->size = size;
}
void heapwatch_free(heapwatch_t *watch, uint32_t ptr)
{
    if (ptr == 0)
    {
        return;
    }
    for (uint32_t i = 0; i < watch->index; i++)
    {
        if (watch->recs[i].ptr == ptr)
        {
            watch->recs[i].ptr = 0;
            watch->recs[i].size = 0;
            return;
        }
    }
    kpanic("DOUBLE FREE");
}

void heap_check(heap_t *heap)
{
    hheader_t *start = heap->start;
    hheader_t *end = heap->start + heap->size;
    hheader_t *prev = NULL;
    hheader_t *cur = heap->start;
    hheader_t *next = NULL;

    while (1)
    {
        if (cur->is_hole > 1 || cur->prev > cur)
        {
            kpanic("corrupted heap (1)");
        }
        prev = cur->prev;
        if (prev >= start)
        {
            if ((uint32_t)prev + sizeof(hheader_t) + prev->size != (uint32_t)cur)
            {
                kpanic("corrupted heap (2)");
            }
            if (prev->is_hole && cur->is_hole)
            {
                kpanic("corrupted heap (3)");
            }
        }
        next = (hheader_t *)((uint32_t)cur + sizeof(hheader_t) + cur->size);
        if (next >= end)
        {
            break;
        }
        else
        {
            if (next->prev != cur)
            {
                kpanic("corrupted heap (4)");
            }

            if (cur->is_hole && next->is_hole)
            {
                kpanic("corrupted heap (5)");
            }
        }
        cur = next;
    }
}