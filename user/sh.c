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
        if(strcmp(command,"cd") == 0)
        {
            if(arg_idx >= 2)
            {
                setcwd(arglist[1]);
            }
            return;
        }
        int pid = fork();
        if(pid)
        {
            int16_t status;
            wait(&status);
            if(status != 0)
            {
                printf("sh: command ended with error!\n");
            }
        }
        else{
            int rsl = exec(command,arglist);
            if(rsl)
            {
                printf("sh: failed to execute command!\n");
                exit(0);
            }
        }
    }
}

void parse_input(int* arg_idx,char* buffer,char** arglist)
{
    *arg_idx = 0;    
    char* arg_ptr = NULL;
    for(int i=0;;i++)
    {
        if(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == 0)
        {
            int flag = buffer[i] == 0 ? 1 : 0;
            if(arg_ptr)
            {
                buffer[i] = 0;
                arglist[(*arg_idx)++] = arg_ptr;
                arg_ptr = NULL;
            }
            if(flag)
            {
                break;
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
    char cwd[BUFFER_SIZE];
    int arg_idx = 0;
    while (1)
    {
        for(int i=0;i<ARG_MAX_COUNT;i++)
        {
            arglist[i] = NULL;
        }
        getcwd(cwd);
        printf("%s$ ",cwd);
        int len = read(STDIN, buffer, BUFFER_SIZE);
        if(!len)
        {
            break;
        }
        buffer[len] = 0;
        parse_input(&arg_idx,buffer,arglist);
        execute_command(arg_idx,arglist);    
    }
    return 0;
}