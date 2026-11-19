#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t child1 = fork();

    if (child1 == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execlp("ls", "ls", "-l", NULL);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }

    pid_t child2 = fork();

    if (child2 == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        execlp("wc", "wc", "-l", NULL);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    return EXIT_SUCCESS;
}
