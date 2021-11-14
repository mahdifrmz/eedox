#include <ordlist.h>

uint32_t ordlist_insert(ordlist_t *ordlist, void *elem)
{
    uint32_t idx = ordlist->size;
    while (idx > 0 && ((ordlist->compare(ordlist->array[idx - 1], elem) == 1 && ordlist->order == ORDER_ASCENDING) ||
                       (ordlist->compare(ordlist->array[idx - 1], elem) == -1 && ordlist->order == ORDER_DESCENDING)))
    {
        ordlist->array[idx] = ordlist->array[idx - 1];
        idx--;
    }
    ordlist->array[idx] = elem;
    ordlist->size++;
    return idx;
}
void *ordlist_remove(ordlist_t *ordlist, int32_t idx)
{
    void *ret = ordlist->array[idx];
    for (uint32_t i = idx; i < ordlist->size; i++)
    {
        ordlist->array[i] = ordlist->array[i + 1];
    }
    ordlist->size--;
    return ret;
}
void *ordlist_get(ordlist_t *ordlist, int32_t idx)
{
    return ordlist->array[idx];
}
uint32_t ordlist_size(ordlist_t *ordlist)
{
    return ordlist->size;
}
void ordlist_place(ordlist_t *ordlist, void *arr, uint32_t max_size, order_t order, ordlist_compare compare)
{
    ordlist->array = arr;
    ordlist->max_size = max_size;
    ordlist->order = order;
    ordlist->size = 0;
    ordlist->compare = compare;
}
void ordlist_clear(ordlist_t *ordlist)
{
    ordlist->size = 0;
}