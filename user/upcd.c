#include <stdlib.h>

#define BUFFERLEN 512

char f(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    else if(c >= 'A' && c <= 'Z')
    {
        return '-';
    }
    else{
        return '-';
    }
}

int fmain()
{
    printf("daemon started\n");

    const char* mqname_call = "upcmq-call";
    int mq_call[2];
    mqopen(mqname_call,mq_call);

    const char* mqname_service = "upcmq-service";
    int mq_service[2];
    mqopen(mqname_service,mq_service);

    while (1)
    {
        char message[BUFFERLEN];
        int len = read(mq_call[0],message,BUFFERLEN);
        for(int i = 0;i<len;i++)
        {
            message[i] = f(message[i]);
        }
        write(mq_service[1],message,len);
    }
}