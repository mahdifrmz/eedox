#ifndef kstring_H
#define kstring_H

#include <stdint.h>
#include <util.h>
#include <kheap.h>

typedef struct
{
    char *buffer;
    uint32_t cap;
    uint32_t size;
    heap_t *heap;
} kstring_t;

kstring_t kstring_new();
kstring_t kstring_new_s(uint32_t size);
kstring_t kstring_new_h(heap_t *heap);
kstring_t kstring_new_sh(uint32_t size, heap_t *heap);
kstring_t kstring_from(const char *str);
kstring_t kstring_from_h(const char *str, heap_t *heap);

void kstring_push(kstring_t *kstr, char c);
void kstring_append(kstring_t *kstr, const char *str);
void kstring_erase(kstring_t *kstr, uint32_t pos, uint32_t len);
void kstring_insert(kstring_t *kstr, uint32_t pos, const char *str);
void kstring_assign(kstring_t *kstr, const char *str);

uint32_t kstring_size(kstring_t *kstr);
uint32_t kstring_cap(kstring_t *kstr);
char *kstring_str(kstring_t *kstr);

void kstring_resize(kstring_t *kstr, uint32_t size);
void kstring_fit(kstring_t *kstr);
void kstring_free(kstring_t *kstr);
void kstring_clear(kstring_t *kstr);

#endif