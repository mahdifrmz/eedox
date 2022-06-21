#include <stdlib.h>

int fmain(int argc, char** argv)
{
    const char* mqname_call = "upcmq-call";
    int mq_call[2];
    mqopen(mqname_call,mq_call);

    const char* mqname_service = "upcmq-service";
    int mq_service[2];
    mqopen(mqname_service,mq_service);

    for(int i=1;i<argc;i++)
    {
        char* param = argv[i];

        int len = strlen(param);
        char buffer [len + 1];
        buffer[len]=0;
        strcpy(buffer,param);
        
        write(mq_call[1],buffer,len);
        read(mq_service[0],buffer,len);
        printf("%s\n",buffer);
    }
    return 0;
}