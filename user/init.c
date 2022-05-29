#include <stdlib.h>

int main()
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
    exec("/shell");
    exit(0);
    return 0;
}