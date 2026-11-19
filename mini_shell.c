#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_ARGS 64
#define MAX_LINE 1024

void execute_command(char *args[], int background);
void parse_and_execute(char *line);

int main(void)
{
    char line[MAX_LINE];

    signal(SIGINT, SIG_IGN);

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (strncmp(line, "cd ", 3) == 0) {
            char *path = line + 3;
            if (chdir(path) == -1) {
                perror("cd");
            }
            continue;
        }

        parse_and_execute(line);
    }

    return EXIT_SUCCESS;
}

void parse_and_execute(char *line)
{
    char *pipe_pos = strchr(line, '|');

    if (pipe_pos != NULL) {
        *pipe_pos = '\0';
        char *left = line;
        char *right = pipe_pos + 1;

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        pid_t child1 = fork();
        if (child1 == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            char *args[MAX_ARGS];
            int i = 0;
            char *token = strtok(left, " ");
            while (token != NULL && i < MAX_ARGS - 1) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        pid_t child2 = fork();
        if (child2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            char *args[MAX_ARGS];
            int i = 0;
            char *token = strtok(right, " ");
            while (token != NULL && i < MAX_ARGS - 1) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
        return;
    }

    char *redirect_pos = strchr(line, '>');

    if (redirect_pos != NULL) {
        *redirect_pos = '\0';
        char *cmd = line;
        char *filename = redirect_pos + 1;

        while (*filename == ' ') filename++;

        pid_t pid = fork();
        if (pid == 0) {
            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("open");
                _exit(EXIT_FAILURE);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            char *args[MAX_ARGS];
            int i = 0;
            char *token = strtok(cmd, " ");
            while (token != NULL && i < MAX_ARGS - 1) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        waitpid(pid, NULL, 0);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        char *args[MAX_ARGS];
        int i = 0;
        char *token = strtok(line, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        signal(SIGINT, SIG_DFL);
        execvp(args[0], args);
        perror("execvp");
        _exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}
