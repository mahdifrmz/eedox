#include <stdlib.h>
#include <llist.h>

#define BUFFER_SIZE 1024

char* readline()
{
    llist_t chlist = llist_create();
    int strlen = 0;
    while(1)
    {
        char c;
        int rsl = read(STDIN,&c,1);
        if(!rsl) break;
        strlen++;
        llist_bpush(&chlist,(int)c);
        if(c == '\n') break;
    }
    char* line = malloc(strlen + 1);
    line[strlen] = 0;
    for(int i=0;i<strlen;i++)
    {
        line[i] = (char)llist_fpop(&chlist);
    }
    return line;
}

int builtins(char** arglist,int argslen)
{
    const char* command = arglist[0];
    if(strcmp(command,"exit") == 0)
    {
        exit(0);
    }
    if(strcmp(command,"cd") == 0)
    {
        if(argslen >= 2)
        {
            if(setcwd(arglist[1]) != 0)
            {
                printf("sh: directory '%s' does not exist\n",arglist[1]);
            }
        }
        return 1;
    }
    return 0;
}

void execute_command(char** arglist, int argslen)
{
    if(argslen == 0)return;
    if(builtins(arglist,argslen)) return;
    int pid = fork();
    if(pid)
    {
        int16_t status;
        wait(&status);
        if(status != 0)
        {
            printf("sh: command ended with error!\n");
        }
        free(arglist);
    }
    else{
        const char* command = arglist[0];
        int rsl = exec(command,arglist);
        if(rsl < 0)
        {
            if(rsl == -9 /* kernel error code */)
            {
                printf("sh: file '%s' not executable!\n",command);
            }
            else{
                printf("sh: executable not found!\n");
            }
            exit(0);
        }
    }
}

llist_t parse_input(char* line)
{
    llist_t tokens = llist_create();
    char* arg_ptr = NULL;
    for(int i=0;;i++)
    {
        if(line[i] == ' ' || line[i] == '\n' || line[i] == 0)
        {
            int flag = line[i] == 0 ? 1 : 0;
            if(arg_ptr)
            {
                line[i] = 0;
                llist_bpush(&tokens,(int)arg_ptr);
                arg_ptr = NULL;
            }
            if(flag)
            {
                break;
            }
        }
        else if (!arg_ptr)
        {
            arg_ptr = line + i;
        }
    }
    return tokens;
}

int fmain()
{
    char cwd[BUFFER_SIZE];
    while (1)
    {
        getcwd(cwd);
        printf("%s$ ",cwd);
        char* line = readline();
        llist_t tokens = parse_input(line);
        
        int argslen = llist_size(&tokens);
        char** arglist = malloc((argslen + 1) * sizeof(char*));
        for(int i=0;i<argslen;i++)
        {
            arglist[i] = (char*)llist_fpop(&tokens);
        }
        arglist[argslen] = NULL;

        execute_command(arglist,argslen);
        free(line);
    }
    return 0;
}