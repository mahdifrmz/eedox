#include <stdlib.h>

int fmain(int argc, char** argv)
{
    int status = 0;
    if(argc == 1)
    {
        printf("usage: state [FILES...]\n");
    }
    else for(int i=1;i<argc;i++){
        stat_t s;
        int rsl = stat(argv[i],&s);
        if(rsl == 0)
        {
            printf("%s:%s size=%u blocks=%u\n",s.isdir?"dir":"file",argv[i],s.size,s.blocks);
        }
        else{
            status = 1;
            printf("invalid path: '%s'",argv[i]);
        }
    }
    return status;
}