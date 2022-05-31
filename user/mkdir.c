#include <stdlib.h>

void error_message(const char* path)
{
    printf("mkdir: failed to create directory '%s'\n",path);
}

int fmain(int argc ,char** argv)
{
    int status = 0;
    if(argc == 1)
    {
        printf("mkdir: missing directory name\n");
        status = 1;
    }
    else for(int i=1;i<argc;i++)
    {
        int rsl = opendir(argv[i]);
        if(rsl > 0)
        {
            close(rsl);
            error_message(argv[i]);
            status = 1;
        }
        else{
            int rsl = mkdir(argv[i]);
            if(rsl != 0)
            {
                error_message(argv[i]);
                status = 1;
            }
        }
    }
    return status;
}