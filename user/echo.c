#include <stdlib.h>

int fmain(int argc,const char** argv)
{
    for(int i=1;i<argc;i++)
    {
        printf("%s ",argv[i]);
    }
    printf("\n");
    return 0;
}