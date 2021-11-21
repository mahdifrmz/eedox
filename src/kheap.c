#include <kheap.h>
#include <util.h>

int8_t compare(void *a, void *b)
{
    uint32_t as = ((hheader_t *)a)->size;
    uint32_t bs = ((hheader_t *)b)->size;
    if (as == bs)
    {
        return 0;
    }
    else if (as > bs)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

void heap_init(heap_t *heap, void *start, uint32_t size, uint32_t index_size, uint8_t readonly, uint8_t supervisor)
{
    heap->size = size;
    heap->readonly = readonly;
    heap->supervisor = supervisor;

    ordlist_place(&heap->index, (void *)start, index_size, ORDER_ASCENDING, compare);
    start += index_size;

    if ((uint32_t)start % 1000 != 0)
    {
        start = (void *)((uint32_t)start & 0xFFFFF000);
        start += 0x1000;
    }

    heap->start = start;
    hheader_t *hole = (hheader_t *)start;
    hole->is_hole = 1;
    hole->size = size;
    hole->prev = 0x0;
    ordlist_insert(&heap->index, hole);
}

uint32_t alignment(uint32_t x)
{
    if (x % 0x1000 != 0)
    {
        x &= 0xFFFFF000;
        x += 0x1000;
    }
    return x;
}

uint32_t alignment_o(uint32_t x, uint32_t offset)
{
    if (x % 0x1000 > offset)
    {
        x &= 0xFFFFF000;
        x += (0x1000 + offset);
    }
    else
    {
        x &= 0xFFFFF000;
        x += offset;
    }
    return x;
}

void heap_low_split(heap_t *heap, hheader_t *hole, int32_t size)
{
    hheader_t *newhole = (hheader_t *)((uint32_t)hole - size);
    newhole->size = size - sizeof(hheader_t);
    hole->size -= size;

    newhole->prev = hole->prev;
    hole->prev = newhole;

    ordlist_insert(&heap->index, newhole);
    newhole->is_hole = 1;
}

hheader_t *hheader_next(hheader_t *head)
{
    return (hheader_t *)((uint32_t)head + sizeof(hheader_t) + head->size);
}

uint8_t hheader_isvalid(hheader_t *head, heap_t *heap)
{
    return (uint32_t)head < (uint32_t)heap->start + heap->size;
}

void heap_high_split(heap_t *heap, hheader_t *hole, uint32_t size)
{
    hheader_t *next = (hheader_t *)((uint32_t)hole + sizeof(hheader_t) + hole->size);
    hheader_t *newhole = (hheader_t *)((uint32_t)next - size);
    newhole->size = size - sizeof(hheader_t);
    hole->size -= size;
    newhole->prev = hole;
    if (hheader_isvalid(next, heap))
    {
        next->prev = newhole;
    }
    ordlist_insert(&heap->index, newhole);
    newhole->is_hole = 1;
}

uint32_t heap_hole_index(heap_t *heap, hheader_t *head)
{
    return bin_search((uint32_t *)heap->index.array, heap->index.size, (uint32_t)head);
}

void heap_merge(heap_t *heap, hheader_t *hole)
{
    hheader_t *next = hheader_next(hole);
    if (hheader_isvalid(next, heap) && next->is_hole)
    {
        hheader_t *next_next = hheader_next(next);
        if (hheader_isvalid(next_next, heap))
        {
            next_next->prev = hole;
        }
        ordlist_remove(&heap->index, heap_hole_index(heap, next));
        hole->size += (next->size + sizeof(hheader_t));
    }
}

void *heap_alloc(heap_t *heap, uint32_t size, uint8_t align)
{
    if (size == 0)
    {
        return 0x0;
    }
    // find the best hole
    uint32_t best_index = 0;
    hheader_t *best_hole;
    hheader_t *newaddr;
    uint32_t extrasize;
    uint32_t effsize;
    while (best_index < ordlist_size(&heap->index))
    {
        best_hole = ordlist_get(&heap->index, best_index);
        if (align)
        {
            newaddr = (hheader_t *)alignment_o((uint32_t)best_hole, 0x1000 - sizeof(hheader_t));
            extrasize = (uint32_t)newaddr - (uint32_t)best_hole;
            if (best_hole->size > extrasize)
            {
                effsize = best_hole->size - extrasize;
                if (effsize >= size)
                {
                    break;
                }
            }
        }
        else
        {
            if (best_hole->size >= size)
            {
                break;
            }
        }
        best_index++;
    }
    if (best_index == ordlist_size(&heap->index))
    {
        return 0x0;
    }
    best_hole->is_hole = 0;
    ordlist_remove(&heap->index, best_index);
    if (align)
    {
        *newaddr = *best_hole;
        best_hole = newaddr;

        if (extrasize > sizeof(hheader_t))
        {
            heap_low_split(heap, best_hole, extrasize);
        }
        else if (best_hole->prev != 0x0)
        {
            best_hole->prev->size += extrasize;
        }
        else
        {
            best_hole->prev = (hheader_t *)0x1;
        }
    }
    if (best_hole->size - size > sizeof(hheader_t))
    {
        heap_high_split(heap, best_hole, best_hole->size - size);
    }
    return (hheader_t *)((uint32_t)best_hole + sizeof(hheader_t));
}

void heap_free(heap_t *heap, void *ptr)
{
    hheader_t *block = (hheader_t *)((uint32_t)ptr - sizeof(hheader_t));
    block->is_hole = 1;
    ordlist_insert(&heap->index, block);
    heap_merge(heap, block);
    if (block->prev && block->prev->is_hole)
    {
        if ((uint32_t)block->prev == 0x1)
        {
            hheader_t *block_rep = heap->start;
            *block_rep = *block;
            block_rep->size += ((uint32_t)block - (uint32_t)heap->start);
            block = block_rep;
        }
        else
        {
            block = block->prev;
            heap_merge(heap, block);
        }
    }
}