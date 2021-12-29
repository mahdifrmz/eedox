#include <stdlib.h>

int main()
{
    int32_t pid = fork();
    if (pid != 0)
    {
        while (1)
        {
            int16_t s;
            wait(&s);
        }
    }
    exec("/shell");
    exit(0);
    return 0;
}