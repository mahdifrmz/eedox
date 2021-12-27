#include <stdlib.h>
#include <../src/util.h>

int main()
{
    const char *message = "hello world!\n";
    write(STDOUT, message, strlen(message));
    exit();
    return 0;
}