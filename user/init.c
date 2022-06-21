#include <stdlib.h>

int fmain()
{
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