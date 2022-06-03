#ifndef PIPE_H
#define PIPE_H

#include <kqueue.h>
#include <lock.h>
#include <kutil.h>
#include <task.h>

typedef struct{
    kqueue_t list;
    uint32_t reader_count;
    uint32_t writer_count;
    ksemaphore_t lock;
} pipe_t;

void pipe_write(pipe_t* pipe,const char* buffer,uint32_t len);
uint32_t pipe_read(pipe_t* pipe,char* buffer,uint32_t len);
pipe_t pipe_new();
void pipe_destroy(pipe_t* pipe);
void pipe_close_rd(pipe_t* pipe);
void pipe_close_wr(pipe_t* pipe);

#endif