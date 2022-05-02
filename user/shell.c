#include <stdlib.h>

int main()
{
    char buffer[256];
    while (1)
    {
        printf("sh->");
        int len = read(STDIN, buffer, 256);
        if (len > 1)
        {
            write(STDOUT, buffer, len);
        }
    }
    exit(0);
    return 0;
}