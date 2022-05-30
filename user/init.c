#include <stdlib.h>

int fmain()
{
    int32_t shell_pid = fork();
    if (shell_pid != 0)
    {
        while (1)
        {
            int16_t s;
            int32_t pid = wait(&s);
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
    char* args [2] = {"/sh",NULL};
    exec("/sh",args);
    exit(0);
    return 0;
}