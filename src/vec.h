#ifndef VEC_H
#define VEC_H

#include <stdint.h>
#include <kheap.h>
#include <util.h>

typedef struct
{
    uint32_t *buffer;
    uint32_t size;
    uint32_t cap;
    heap_t *heap;
} vec_t;

vec_t vec_new();
vec_t vec_new_s(uint32_t size);
vec_t vec_new_h(heap_t *heap);
vec_t vec_new_sh(uint32_t size, heap_t *heap);

void vec_push(vec_t *vec, uint32_t val);
uint32_t vec_pop(vec_t *vec);
void vec_insert(vec_t *vec, uint32_t pos, uint32_t val);
void vec_erase(vec_t *vec, uint32_t pos, uint32_t len);
vec_t vec_copy(vec_t *vec);

uint32_t vec_size(vec_t *vec);
uint32_t vec_cap(vec_t *vec);

void vec_resize(vec_t *vec, uint32_t size);
void vec_fit(vec_t *vec);
void vec_free(vec_t *vec);
void vec_clear(vec_t *vec);

#endif