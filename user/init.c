#include <stdlib.h>

void child(const char* path)
{
    int pid = fork();
    if(pid == 0)
    {
        char* bin = (char*)path;
        char* args [2] = {bin,NULL};
        exec(path,args);
    }
}

/* BROKEN */
void run_services()
{
    const int list_max_size = 512;
    char list [list_max_size];
    int list_fd = open("/etc/services",0);
    int list_len = read(list_fd,list,list_max_size);
    list[list_len] = 0;
    char* ptr = NULL;
    for(int i=0;i<list_len;i++)
    {
        char c = list[i];
        if(c == ' ' || c == '\n')
        {
            if(ptr && list + i > ptr)
            {
                list[i] = 0;
                printf("%s started\n",ptr);
                child(ptr);
                ptr=NULL;
            }
        }
        else{
            if(!ptr)
            {
                ptr = list + i;
            }
        }
    }
}

int fmain()
{
    child("/etc/upcd");
    setcwd("/home");
    int shell_pid = fork();
    if (shell_pid != 0)
    {
        while (1)
        {
            int16_t s;
            int pid = wait(&s);
            if(pid == shell_pid)
            {
                pid = fork();
                if (!pid)
                {
                    break;
                }
                else{
                    shell_pid = pid;
                }
            }
        }
    }
    char* args [2] = {"/bin/sh",NULL};
    exec("/bin/sh",args);
    exit(0);
    return 0;
}