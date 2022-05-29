#include <stdlib.h>

#define BUFFER_SIZE 256

int fmain()
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        printf("sh->");
        int len = read(STDIN, buffer, BUFFER_SIZE);
        buffer[len] = 0;
        if(strcmp(buffer,"exit\n") == 0)
        {
            return 0;
        }
        else if (len > 1)
        {
            write(STDOUT, buffer, len);
        }
    }
    return 0;
}