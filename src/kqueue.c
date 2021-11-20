#include <kqueue.h>
#include <util.h>

kqueue_t kqueue_new()
{
    return kqueue_new_h(&kernel_heap);
}

kqueue_t kqueue_new_h(heap_t *heap)
{
    kqueue_t queue;
    queue.size = 0;
    queue.head = NULL;
    queue.tail = NULL;
    queue.heap = heap;
    return queue;
}

void kqueue_push(kqueue_t *queue, uint32_t value)
{
    kqueue_ele *ele = heap_alloc(queue->heap, sizeof(kqueue_ele), 0);
    ele->value = value;
    ele->next = NULL;
    if (queue->size)
    {
        queue->tail->next = ele;
    }
    else
    {
        queue->head = ele;
    }
    queue->tail = ele;
    queue->size++;
}

uint32_t kqueue_pop(kqueue_t *queue)
{
    kqueue_ele ele = *queue->head;
    heap_free(queue->heap, queue->head);
    queue->head = ele.next;
    if (!ele.next)
    {
        queue->tail = NULL;
    }
    queue->size--;
    return ele.value;
}

uint32_t kqueue_peek(kqueue_t *queue)
{
    return queue->head->value;
}

void kqueue_clear(kqueue_t *queue)
{
    kqueue_ele *ele = queue->head;
    while (ele)
    {
        kqueue_ele *next = ele;
        heap_free(queue->heap, ele);
        ele = next;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}