#ifndef KHEAP_H
#define KHEAP_H

#define KHEAP_INDEX_SIZE 0x20000

#include <ordlist.h>

typedef struct hheader_t hheader_t;

struct hheader_t
{
    uint8_t is_hole;
    uint32_t size;
    hheader_t *prev;
} __attribute__((packed));

typedef struct
{
    ordlist_t index;
    void *start;
    uint32_t size;
    uint8_t readonly;
    uint8_t supervisor;
} heap_t;

extern heap_t kernel_heap;

void heap_init(heap_t *heap, void *start, uint32_t size, uint32_t index_size, uint8_t readonly, uint8_t supervisor);
void *heap_alloc(heap_t *heap, uint32_t size, uint8_t align);
void heap_free(heap_t *heap, void *ptr);

#endif