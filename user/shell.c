#include <stdlib.h>

int main()
{
    char buffer[256];
    while (1)
    {
        printf("sh(%u)->", getpid());
        int len = read(STDIN, buffer, 256);
        write(STDOUT, buffer, len);
    }
    exit(0);
    return 0;
}