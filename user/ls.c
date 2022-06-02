#include <stdlib.h>

#define BUFFER_SIZE 256

int ls(const char* address)
{
    int fd = opendir(address);
    if(fd < 0)
    {
        return 1;
    }
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
    return 0;
}

int fmain(int argc,const char** argv)
{
    int status = 0;
    char cwd [BUFFER_SIZE];
    if(argc == 1)
    {
        getcwd(cwd);
        status |= ls(cwd);
    }
    else{
        for(int i=1;i<argc;i++)
        {
            status |= ls(argv[i]);
        }
    }
    return status;
}