#include <pipe.h>

void pipe_write(pipe_t* pipe,const char* buffer,uint32_t len)
{
    char* buf = (char*)kmalloc(len+1);
    memcpy(buf,buffer,len);
    buf[len]=0;
    kqueue_push(&pipe->list,(uint32_t)buf);
    ksemaphore_signal(&pipe->lock);
}

uint32_t pipe_read(pipe_t* pipe,char* buffer,uint32_t len)
{
    ksemaphore_wait(&pipe->lock);
    char *input = (char *)kqueue_peek(&pipe->list);
    uint32_t input_len = strlen(input);
    uint32_t count = min(len, input_len);
    memcpy(buffer, input, count);
    if (len < input_len)
    {
        uint32_t extra_len = input_len - len;
        memcpy(input, input + len, extra_len);
        input[extra_len] = 0;
        ksemaphore_signal(&pipe->lock);
    }
    else
    {
        kqueue_pop(&pipe->list);
        kfree(input);
    }
    return count;
}

pipe_t pipe_new()
{
    pipe_t pipe;
    pipe.reader_count = 1;
    pipe.writer_count = 1;
    pipe.list = kqueue_new();
    ksemaphore_init(&pipe.lock,0);
    return pipe;
}

void pipe_destroy(pipe_t* pipe)
{
    while(pipe->list.size)
    {
        kfree((void*)kqueue_pop(&pipe->list));
    }
    kfree(pipe);
}

void pipe_close_rd(pipe_t* pipe)
{
    pipe->reader_count--;
    if(!pipe->reader_count && !pipe->writer_count)
    {
        pipe_destroy(pipe);
    }
}

void pipe_close_wr(pipe_t* pipe)
{
    pipe->reader_count--;
    if(!pipe->reader_count && !pipe->writer_count)
    {
        pipe_destroy(pipe);
    }
}