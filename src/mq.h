#ifndef MQ_H
#define MQ_H

#include <lock.h>
#include <pipe.h>

typedef struct
{
    vec_t names;
    vec_t pipes;
} mqlist_t;

mqlist_t mqlist_new();
pipe_t* mqlist_open(mqlist_t *list,const char* name);

#endif