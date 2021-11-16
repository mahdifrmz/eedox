#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>

typedef struct
{
    uint8_t *start;
    uint32_t len;
} bitset_t;

void bitset_init(bitset_t *bs, void *start, uint32_t len);
uint8_t bitset_get(bitset_t *bs, uint32_t index);
void bitset_set(bitset_t *bs, uint32_t index, uint8_t val);
int32_t bitset_first_unset(bitset_t *bs);

#endif