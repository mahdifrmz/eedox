#ifndef HEAPWATCH_H
#define HEAPWATCH_H

#include <stdint.h>
#include <kheap.h>

typedef struct
{
    uint32_t ptr;
    uint32_t size;
} watchrec_t;

typedef struct
{
    watchrec_t recs[1024];
    uint32_t index;
} heapwatch_t;

void heapwatch_init(heapwatch_t *watch);
void heapwatch_alloc(heapwatch_t *watch, uint32_t ptr, uint32_t size);
void heapwatch_free(heapwatch_t *watch, uint32_t ptr);

void heap_check(heap_t *heap);

#endif