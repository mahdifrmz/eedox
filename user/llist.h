#ifndef LLIST_H
#define LLIST_H

typedef struct llist_ele llist_ele;

struct llist_ele
{
    int value;
    llist_ele* next;
    llist_ele* prev;
};

typedef struct
{
    int size;
    llist_ele* head;
    llist_ele* tail;
} llist_t;

llist_t llist_create();
int llist_size(const llist_t* list);
void llist_clear(llist_t* list);
int llist_fpop(llist_t* list);
int llist_bpop(llist_t* list);
void llist_fpush(llist_t* list,int value);
void llist_bpush(llist_t* list,int value);
int llist_fpeek(const llist_t* list);
int llist_bpeek(const llist_t* list);

#endif