#include <stdlib.h>
#include <../src/util.h>

void pwd()
{
    const char *message = "CWD=";
    write(STDOUT, message, strlen(message));
    char cwd[32];
    getcwd(cwd);
    write(STDOUT, cwd, strlen(cwd));
}
int main()
{
    pwd();
    setcwd("/ffol");
    pwd();
    exit();
    return 0;
}