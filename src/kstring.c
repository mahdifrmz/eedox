#include <kstring.h>
#define kstring_INIT_CAP 4

extern heap_t glb_heap;

void *kstring_alloc(kstring_t *kstr, uint32_t size)
{
    return heap_alloc(kstr->heap, size + 1, 0);
}

kstring_t kstring_new_sh(uint32_t size, heap_t *heap)
{
    kstring_t kstr;
    kstr.size = 0;
    kstr.cap = size;
    kstr.heap = heap;
    kstr.buffer = kstring_alloc(&kstr, kstr.cap);
    return kstr;
}

kstring_t kstring_new_s(uint32_t size)
{
    return kstring_new_sh(size, &glb_heap);
}

kstring_t kstring_new_h(heap_t *heap)
{
    return kstring_new_sh(kstring_INIT_CAP, heap);
}

kstring_t kstring_new()
{
    return kstring_new_sh(kstring_INIT_CAP, &glb_heap);
}

kstring_t kstring_from_h(const char *str, heap_t *heap)
{
    uint32_t size = strlen(str);
    uint32_t cap = kstring_INIT_CAP;
    while (cap < size)
    {
        cap *= 2;
    }
    kstring_t kstr = kstring_new_sh(cap, heap);
    kstr.size = size;
    strcpy(kstr.buffer, str);
    kstr.buffer[size] = 0;
    return kstr;
}

kstring_t kstring_from(const char *str)
{
    return kstring_from_h(str, &glb_heap);
}

void kstring_push(kstring_t *kstr, char c)
{
    if (kstr->size == kstr->cap)
    {
        kstring_resize(kstr, kstr->cap * 2);
    }
    kstr->buffer[kstr->size++] = c;
    kstr->buffer[kstr->size] = 0;
}

void kstring_append(kstring_t *kstr, const char *str)
{
    uint32_t str_size = strlen(str);
    uint32_t new_size = kstr->size + str_size;
    if (new_size > kstr->cap)
    {
        uint32_t new_cap = kstr->cap;
        while (new_size > new_cap)
        {
            new_cap *= 2;
        }
        kstring_resize(kstr, new_cap);
    }
    strcpy(kstr->buffer + kstr->size, str);
    kstr->size = new_size;
    kstr->buffer[kstr->size] = 0;
}

void kstring_erase(kstring_t *kstr, uint32_t pos, uint32_t len)
{
    if (pos + len >= kstr->size)
    {
        if (pos < kstr->size)
        {
            kstr->size = pos;
            kstr->buffer[pos] = 0;
        }
    }
    else
    {
        int32_t rem = len;
        rem = -rem;
        memshift(kstr->buffer + pos + len, rem, kstr->size - pos - len);
        kstr->size -= len;
        kstr->buffer[kstr->size] = 0;
    }
}

void kstring_insert(kstring_t *kstr, uint32_t pos, const char *str)
{
    if (pos >= kstr->size)
    {
        kstring_append(kstr, str);
        return;
    }

    uint32_t str_size = strlen(str);
    uint32_t new_size = kstr->size + str_size;
    if (new_size > kstr->cap)
    {
        uint32_t new_cap = kstr->cap;
        while (new_size > new_cap)
        {
            new_cap *= 2;
        }
        kstring_resize(kstr, new_cap);
    }
    memshift(kstr->buffer + pos, str_size, kstr->size - pos);
    strcpy(kstr->buffer + pos, str);
    kstr->buffer[(kstr->size += str_size)] = 0;
}

void kstring_assign(kstring_t *kstr, const char *str)
{
    kstring_free(kstr);
    *kstr = kstring_from_h(str, kstr->heap);
}

uint32_t kstring_size(kstring_t *kstr)
{
    return kstr->size;
}

uint32_t kstring_cap(kstring_t *kstr)
{
    return kstr->cap;
}

char *kstring_str(kstring_t *kstr)
{
    return kstr->buffer;
}

void kstring_resize(kstring_t *kstr, uint32_t size)
{
    if (size >= kstr->size)
    {
        char *newbuffer = kstring_alloc(kstr, size);
        strcpy(newbuffer, kstr->buffer);
        newbuffer[kstr->size] = 0;
        heap_free(kstr->heap, kstr->buffer);
        kstr->buffer = newbuffer;
        kstr->cap = size;
    }
}

void kstring_fit(kstring_t *kstr)
{
    kstring_resize(kstr, kstr->size + 1);
}
void kstring_free(kstring_t *kstr)
{
    heap_free(kstr->heap, kstr->buffer);
}
void kstring_clear(kstring_t *kstr)
{
    kstr->size = 0;
    kstr->buffer[0] = 0;
}