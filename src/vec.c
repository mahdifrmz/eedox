#include <vec.h>

#define VEC_INIT_CAP 4

extern heap_t kernel_heap;

void *vec_alloc(vec_t *vec, uint32_t size)
{
    return heap_alloc(vec->heap, size * sizeof(uint32_t), 0);
}

vec_t vec_new_sh(uint32_t size, heap_t *heap)
{
    vec_t vec;
    vec.size = 0;
    vec.cap = size;
    vec.heap = heap;
    vec.buffer = vec_alloc(&vec, vec.cap);
    return vec;
}

vec_t vec_new_s(uint32_t size)
{
    return vec_new_sh(size, &kernel_heap);
}

vec_t vec_new_h(heap_t *heap)
{
    return vec_new_sh(VEC_INIT_CAP, heap);
}

vec_t vec_new()
{
    return vec_new_sh(VEC_INIT_CAP, &kernel_heap);
}

void vec_push(vec_t *vec, uint32_t val)
{
    if (vec->size == vec->cap)
    {
        vec_resize(vec, vec->cap * 2);
    }
    vec->buffer[vec->size++] = val;
}

uint32_t vec_pop(vec_t *vec)
{
    return vec->buffer[--vec->size];
}

void vec_erase(vec_t *vec, uint32_t pos, uint32_t len)
{
    if (pos + len >= vec->size)
    {
        if (pos < vec->size)
        {
            vec->size = pos;
        }
    }
    else
    {
        int32_t rem = len;
        rem = -rem;
        memshift(vec->buffer + pos + len, rem * sizeof(uint32_t), (vec->size - pos - len) * sizeof(uint32_t));
        vec->size -= len;
    }
}

void vec_insert(vec_t *vec, uint32_t pos, uint32_t val)
{
    if (pos >= vec->size)
    {
        vec_push(vec, val);
        return;
    }
    if (vec->size == vec->cap)
    {
        vec_resize(vec, vec->cap * 2);
    }
    memshift(vec->buffer + pos, sizeof(uint32_t), (vec->size - pos) * sizeof(uint32_t));
    vec->buffer[pos] = val;
    vec->size++;
}

vec_t vec_copy(vec_t *vec)
{
    vec_t newvec = *vec;
    newvec.buffer = vec_alloc(vec, vec->cap);
    memcpy(newvec.buffer, vec->buffer, vec->size * sizeof(uint32_t));
    return newvec;
}

uint32_t vec_size(vec_t *vec)
{
    return vec->size;
}

uint32_t vec_cap(vec_t *vec)
{
    return vec->cap;
}

void vec_resize(vec_t *vec, uint32_t size)
{
    if (size >= vec->size)
    {
        uint32_t *newbuffer = vec_alloc(vec, size);
        memcpy(newbuffer, vec->buffer, vec->size);
        heap_free(vec->heap, vec->buffer);
        vec->buffer = newbuffer;
        vec->cap = size;
    }
}

void vec_fit(vec_t *vec)
{
    vec_resize(vec, vec->size);
}
void vec_free(vec_t *vec)
{
    heap_free(vec->heap, vec->buffer);
}
void vec_clear(vec_t *vec)
{
    vec->size = 0;
}