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
    int status = 0;
    if(argc > 1)
    {
        for(int i=1;i<argc;i++)
        {
            int fd = open(argv[i],0);
            if(fd < 0)
            {
                printf("cat: error opening %s\n",argv[i]);
                status = 1;
            }
            else{
                cat(fd);
            }
        }
    }
    else{
        cat(STDIN);
    }
    return status;
}