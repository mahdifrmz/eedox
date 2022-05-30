#include <stdlib.h>

#define BUFFER_SIZE 1024

void cat(int fd)
{
    char buffer [BUFFER_SIZE];
    while(1)
    {
        int rsl = read(fd,buffer,BUFFER_SIZE);
        if(!rsl)
        {
            break;
        }
        write(STDOUT,buffer,rsl);
    }
}

int fmain(int argc, const char** argv)
{
    if(argc > 1)
    {
        for(int i=1;i<argc;i++)
        {
            int fd = open(argv[i],0);
            cat(fd);
        }
    }
    else{
        cat(STDIN);
    }
    return 0;
}