#include <stdlib.h>

#define BUFFER_SIZE 1024

int wc(int fd)
{
    char buffer[BUFFER_SIZE];
    int count = 0;
    while(1)
    {
        int rsl = read(fd,buffer,BUFFER_SIZE);
        if(rsl == 0)
        {
            break;
        }
        count += rsl;
    }
    return count;
}

int fmain(int argc, char** argv)
{
    int status = 0;
    if(argc == 1)
    {
        printf("%u\n",wc(STDIN));
    }
    else{
        for(int i=1;i<argc;i++)
        {
            int fd = open(argv[i],0);
            if(fd < 0)
            {
                status = 1;
                printf("wc: failed to open file '%s'\n",argv[i]);
            }
            else
            {
                printf("%s : %u\n",argv[i],wc(fd));
            }
        }
    }
    return status;
}