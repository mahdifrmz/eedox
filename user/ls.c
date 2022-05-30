#include <stdlib.h>

#define BUFFER_SIZE 256

void ls(const char* address)
{
    int fd = opendir(address);
    char name [BUFFER_SIZE];
    while(1)
    {
        int rsl = readdir(fd,name);
        if(!rsl)
        {
            break;
        }
        printf("%s\n",name);
    }
    close(fd);
}

int fmain(int argc,const char** argv)
{
    char cwd [BUFFER_SIZE];
    if(argc == 1)
    {
        getcwd(cwd);
        ls(cwd);
    }
    else{
        for(int i=1;i<argc;i++)
        {
            ls(argv[i]);
        }
    }

    return 0;
}