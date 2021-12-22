#ifndef ORDLIST_H
#define ORDLIST_H

#include <stdint.h>
#include <util.h>

typedef enum
{
    ORDER_ASCENDING,
    ORDER_DESCENDING,
} order_t;

typedef struct
{
    uint32_t size;
    uint32_t max_size;
    void **array;
    compare cmp;
    order_t order;
} ordlist_t;

uint32_t ordlist_insert(ordlist_t *ordlist, void *elem);
void ordlist_clear(ordlist_t *ordlist);
void *ordlist_remove(ordlist_t *ordlist, int32_t idx);
void *ordlist_get(ordlist_t *ordlist, int32_t idx);
uint32_t ordlist_size(ordlist_t *ordlist);
void ordlist_place(ordlist_t *ordlist, void *arr, uint32_t max_size, order_t order, compare cmp);
#endif