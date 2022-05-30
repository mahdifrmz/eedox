#include <stdlib.h>

#define BUFFER_SIZE 1024

void copy(int srcfd, int destfd)
{
    char buffer [BUFFER_SIZE];
    while(1)
    {
        int rsl = read(srcfd,buffer,BUFFER_SIZE);
        if(!rsl)
        {
            break;
        }
        write(destfd,buffer,rsl);
    }
}

int fmain(int argc, const char** argv)
{
    if(argc >= 3)
    {
        const char* src = argv[1];
        const char* dest = argv[2];
        int srcfd = open(src,0);
        int destfd = open(dest,3);
        if(srcfd < 0 || destfd < 0)
        {
            printf("cp: failed to copy file '%s' to destination '%s'\n",src,dest);
            return 2;
        }
        else{
            copy(srcfd,destfd);
            close(srcfd);
            close(destfd);
            return 0;
        }
    }
    else{
        printf("usage: cp [src] [dest]\n");
        return 1;
    }
}