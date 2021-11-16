#include <bitset.h>

void bitset_init(bitset_t *bs, void *start, uint32_t len)
{
    bs->start = (uint8_t *)start;
    bs->len = len;
    for (uint32_t i = 0; i < len / 8; i++)
    {
        bs->start[i] = 0x00;
    }
}

uint8_t bitset_get(bitset_t *bs, uint32_t index)
{
    return ((bs->start)[index / 8] & (0x80 >> (index % 8))) ? 1 : 0;
}

void bitset_set(bitset_t *bs, uint32_t index, uint8_t val)
{
    bs->start[index / 8] |= ((val * 0x80) >> (index % 8));
}

int32_t bitset_first_unset(bitset_t *bs)
{
    for (uint32_t i = 0; i < bs->len / 8; i++)
    {
        if (bs->start[i] == 0xff)
        {
            continue;
        }
        for (uint32_t j = 0; j < 8; j++)
        {
            uint32_t idx = i * 8 + j;
            if (!bitset_get(bs, idx))
            {
                return idx;
            }
        }
    }
    return -1;
}