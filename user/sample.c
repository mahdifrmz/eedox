#include <stdlib.h>

int fmain()
{
    int pipe_fds[2];
    pipe(pipe_fds);
    int pid = fork();
    if (pid == 0)
    {
        // child
        close(pipe_fds[1]);
        char c;
        while (read(pipe_fds[0], &c, 1))
        {
            if (c >= 97 && c <= 122)
                c -= 32;
            write(STDOUT, &c, 1);
        }
    }
    else
    {
        // parent
        close(pipe_fds[0]);
        const char *message = "hello world!\n";
        write(pipe_fds[1], message, strlen(message));
        close(pipe_fds[1]);
        short chstat;
        wait(&chstat);
    }
    return 0;
}