#include <stdlib.h>

#define BUFFER_SIZE 1024
#define ARG_MAX_COUNT 16

void execute_command(int arg_idx,char** arglist)
{
    if(arg_idx)
    {
        const char* command = arglist[0];
        if(strcmp(command,"exit") == 0)
        {
            exit(0);
        }
        int pid = fork();
        if(pid)
        {
            int16_t status;
            wait(&status);
            if(status != 0)
            {
                printf("failed to execute command!\n");
            }
        }
        else{
            int rsl = exec(command,arglist);
            if(rsl)
            {
                exit(1);
            }
        }
    }
}

void parse_input(int* arg_idx,char* buffer,char** arglist)
{
    *arg_idx = 0;    
    char* arg_ptr = NULL;
    for(int i=0;buffer[i] != 0;i++)
    {
        if(buffer[i] == ' ' || buffer[i] == '\n')
        {
            if(arg_ptr)
            {
                buffer[i] = 0;
                arglist[(*arg_idx)++] = arg_ptr;
                arg_ptr = NULL;
            }
        }
        else if (!arg_ptr)
        {
            arg_ptr = buffer + i;
        }
    }
}

int fmain()
{
    char buffer[BUFFER_SIZE];
    char* arglist [ARG_MAX_COUNT];
    int arg_idx = 0;
    while (1)
    {
        for(int i=0;i<ARG_MAX_COUNT;i++)
        {
            arglist[i] = NULL;
        }
        printf("sh->");
        int len = read(STDIN, buffer, BUFFER_SIZE);
        buffer[len] = 0;
        parse_input(&arg_idx,buffer,arglist);
        execute_command(arg_idx,arglist);    
    }
    return 0;
}