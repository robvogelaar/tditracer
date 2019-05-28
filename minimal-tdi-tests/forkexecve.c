#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t p, p_wait;
    int status;
    char *cmd[] = {"/bin/ls", "-l", ".", 0};

    // fork
    if ((p = fork()) > 0) {
        // parent
        p_wait = wait(&status);
        printf("%s [%d] exited with %d\n", cmd[0], p_wait, status);
    } else {
        // child process
        // execve
        execve(cmd[0], cmd, 0);
    }

    return 0;
}

