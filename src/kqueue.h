#ifndef KQUEUE_H
#define KQUEUE_H

#include <stdint.h>
#include <kheap.h>

extern heap_t kernel_heap;

typedef struct kqueue_ele kqueue_ele;

struct kqueue_ele
{
    kqueue_ele *next;
    uint32_t value;
};

typedef struct
{
    kqueue_ele *head;
    kqueue_ele *tail;
    uint32_t size;
    heap_t *heap;
} kqueue_t;

kqueue_t kqueue_new();
kqueue_t kqueue_new_h(heap_t *heap);
void kqueue_push(kqueue_t *queue, uint32_t value);
uint32_t kqueue_pop(kqueue_t *queue);
uint32_t kqueue_peek(kqueue_t *queue);
void kqueue_clear(kqueue_t *queue);

#endif