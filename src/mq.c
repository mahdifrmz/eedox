#include <mq.h>

mqlist_t mqlist_new()
{
    mqlist_t mq;
    mq.names = vec_new();
    mq.pipes = vec_new();
    return mq;
}

pipe_t* mqlist_open(mqlist_t *list,const char* name)
{
    uint32_t free_index = (uint32_t)-1;
    for(uint32_t i=0;i<vec_size(&list->names);i++)
    {
        const char* ent_name = (const char*)list->names.buffer[i];
        pipe_t* ent_pipe = (pipe_t*)list->pipes.buffer[i];
        if(strcmp(ent_name,name) == 0)
        {
            if(ent_pipe->dead)
            {
                *ent_pipe = pipe_new();
            }
            return ent_pipe;
        }
        if(free_index == (uint32_t)-1 && ent_pipe->dead)
        {
            free_index = i;
        }
    }
    if(free_index == (uint32_t)-1)
    {
        vec_push(&list->names,0);
        vec_push(&list->pipes,0);
        free_index = list->names.size - 1;
    }
    pipe_t* pipe = (pipe_t*)kmalloc(sizeof(pipe_t));
    *pipe = pipe_new();
    list->names.buffer[free_index] = (uint32_t)name;
    list->pipes.buffer[free_index] = (uint32_t)pipe;
    return pipe;
}
