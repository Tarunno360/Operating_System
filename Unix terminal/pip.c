#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_NUM_PIPES 16
#define MAX_ARGS 64

// Function to split string by delimiter
int split_by_delim(char *line, char **commands, const char *delim) {
    int count = 0;
    char *token = strtok(line, delim);
    while (token != NULL && count < MAX_NUM_PIPES) {
        commands[count++] = token;
        token = strtok(NULL, delim);
    }
    commands[count] = NULL;
    return count;
}

// Split a single command into arguments
void parse_args(char *command, char **args) {
    int i = 0;
    args[i] = strtok(command, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

void execute_pipeline(char **commands, int num_cmds) {
    int i;
    int pipefd[2];
    int prev_fd = -1;
    pid_t pids[MAX_NUM_PIPES];

    for (i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process

            if (i > 0) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (i < num_cmds - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            char *args[MAX_ARGS];
            parse_args(commands[i], args);
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(1);
            }
        } else if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }

        // Parent process
        if (prev_fd != -1) close(prev_fd);
        if (i < num_cmds - 1) {
            close(pipefd[1]);     // close write end in parent
            prev_fd = pipefd[0];  // save read end for next command
        }
    }

    // Parent waits for all children
    for (i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int main() {
    char line[MAX_COMMAND_LENGTH];
    char *commands[MAX_NUM_PIPES];

    while (1) {
        printf("sh> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
        if (strcmp(line, "exit") == 0) break;

        int num_cmds = split_by_delim(line, commands, "|");

        execute_pipeline(commands, num_cmds);
    }

    return 0;
}
