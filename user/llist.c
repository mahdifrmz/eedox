#include <llist.h>
#include <stdlib.h>


llist_t llist_create()
{
    llist_t list;
    list.head = NULL;
    list.tail = NULL;
    list.size = 0;
    return list;
}
int llist_size(const llist_t* list)
{
    return list->size;
}
void llist_clear(llist_t* list)
{
    while(list->size)
        llist_fpop(list);
}
int llist_fpop(llist_t* list)
{
    llist_ele* head = list->head;
    int value = head->value;
    list->head = head->prev;
    if(list->head)
    {
        list->head->next = NULL;
    }
    else{
        list->tail = NULL;
    }
    free(head);
    list->size--;
    return value;
}
int llist_bpop(llist_t* list)
{
    llist_ele* tail = list->tail;
    int value = tail->value;
    list->tail = tail->next;
    if(list->tail)
    {
        list->tail->prev = NULL;
    }
    else{
        list->head = NULL;
    }
    free(tail);
    list->size--;
    return value;
}
void llist_fpush(llist_t* list,int value)
{
    llist_ele* ele = malloc(sizeof(llist_ele));
    ele->value = value;
    ele->next = NULL;
    ele->prev = list->head;
    list->head = ele;
    if(ele->prev)
    {
        ele->prev->next = ele;
    }
    else{
        list->tail = ele;
    }
    list->size++;
}
void llist_bpush(llist_t* list,int value)
{
    llist_ele* ele = malloc(sizeof(llist_ele));
    ele->value = value;
    ele->prev = NULL;
    ele->next = list->tail;
    list->tail = ele;
    if(ele->next)
    {
        ele->next->prev = ele;
    }
    else{
        list->head = ele;
    }
    list->size++;
}
int llist_fpeek(const llist_t* list)
{
    return list->head->value;
}
int llist_bpeek(const llist_t* list)
{
    return list->tail->value;
}