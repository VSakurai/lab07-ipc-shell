#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void handle_sigterm(int sig)
{
    (void)sig;
    const char message[] = "Child: SIGTERM received\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(0);
}

int main(void)
{
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        struct sigaction action = {0};
        action.sa_handler = handle_sigterm;
        sigemptyset(&action.sa_mask);
        sigaction(SIGTERM, &action, NULL);

        while (1) {
            pause();
        }
    }

    sleep(2);
    printf("Parent: sending SIGTERM to child %d\n", pid);
    kill(pid, SIGTERM);

    waitpid(pid, NULL, 0);
    printf("Parent: child terminated\n");

    return EXIT_SUCCESS;
}
