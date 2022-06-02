#ifndef ALLOC_INTERNAL_H
#define ALLOC_INTERNAL_H

#include <alloc.h>

#define SEQUENTIAL 0
#define FIRST_FIT 1
#define BEST_FIT 2

#ifndef BLOCK_FINDER
#define BLOCK_FINDER BEST_FIT
#endif

size_t alignment(size_t size, size_t align);
void copy(void *src, void *dest, size_t len);

typedef struct object_header object_header;
struct object_header
{
    size_t size;
    uint8_t used;
    object_header *next_free;
    object_header *prev_free;
};

object_header *object_header_next(const object_header *o_head);
object_header object_header_new(size_t size);
uint8_t object_header_get_used(const object_header *o_head);
void object_header_set_used(object_header *o_head, uint8_t flag);
size_t object_header_get_size(const object_header *o_head);
void object_header_set_size(object_header *o_head, size_t size);
uint8_t object_header_is_null(const object_header *o_head, const heap_t *mem);
void object_header_merge(object_header *o_head);
object_header *object_header_try_split(object_header *o_head, size_t size, heap_t *mem);
object_header *object_header_from_pointer(void *pointer);
void *object_header_get_pointer(object_header *o_head);

object_header *heap_find_block(heap_t *mem, size_t size);
object_header *heap_find_first_fit(heap_t *mem, size_t size);
object_header *heap_find_best_fit(heap_t *mem, size_t size);
void heap_add_free(heap_t *mem, object_header *header);
void heap_remove_free(heap_t *mem, object_header *header);
uint8_t heap_is_end(const heap_t *mem, const object_header *header);

#endif