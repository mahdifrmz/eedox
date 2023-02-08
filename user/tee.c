#include <stdlib.h>

#define BUFFER_SIZE 1024

int fmain(int argc, const char **argv)
{
    if (argc > 2)
    {
        exit(1);
    }
    int fd = argc == 2 ? open(argv[1], 3) : STDOUT;
    char buffer[BUFFER_SIZE];
    while (1)
    {
        int rsl = read(STDIN, buffer, BUFFER_SIZE);
        if (!rsl)
        {
            break;
        }
        write(fd, buffer, rsl);
    }
    close(fd);
    return 0;
}