#include <stdlib.h>

void pwd()
{
    const char *message = "CWD=";
    write(STDOUT, message, strlen(message));
    char cwd[32];
    getcwd(cwd);
    write(STDOUT, cwd, strlen(cwd));
    write(STDOUT, "\n", 1);
}
void print(const char *message)
{
    write(STDOUT, message, strlen(message));
}
int main()
{
    int16_t exit_code = 33;
    // setcwd("/ffol");
    int32_t pid = fork();
    if (pid != 0)
    {
        while (1)
        {
            int16_t s;
            wait(&s);
            print("KILLED\n");
            if (s == 33)
            {
                print("fine\n");
            }
        }
    }
    pwd();
    exit(exit_code);
    return 0;
}