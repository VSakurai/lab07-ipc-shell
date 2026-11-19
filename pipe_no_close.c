#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        close(pipefd[0]);

        char buffer[128];
        ssize_t n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, n);
        }

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }

    const char message[] = "Message from parent\n";
    write(pipefd[1], message, strlen(message));

    waitpid(pid, NULL, 0);

    return EXIT_SUCCESS;
}
