#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_NUM_ARGS 64

void display_prompt() {
    printf("sh> ");
    fflush(stdout);
}

void read_command(char *command) {
    if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
        perror("fgets failed");
        exit(1);
    }
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }
}

// Split command by space and detect redirection
int parse_command(char *command, char **args, char **input_file, char **output_file, int *append_mode) {
    int i = 0;
    char *token = strtok(command, " ");

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            *input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            *output_file = token;
            *append_mode = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            *output_file = token;
            *append_mode = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }

    args[i] = NULL;
    return i;
}

void execute_command(char **args, char *input_file, char *output_file, int append_mode) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process

        if (input_file) {
            int in_fd = open(input_file, O_RDONLY);
            if (in_fd < 0) {
                perror("Failed to open input file");
                exit(1);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (output_file) {
            int out_fd;
            if (append_mode) {
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }

            if (out_fd < 0) {
                perror("Failed to open output file");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(1);
        }

    } else if (pid > 0) {
        // Parent process
        wait(NULL);
    } else {
        perror("fork failed");
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char *args[MAX_NUM_ARGS];
    char *input_file;
    char *output_file;
    int append_mode;

    while (1) {
        input_file = NULL;
        output_file = NULL;
        append_mode = 0;

        display_prompt();
        read_command(command);

        if (strcmp(command, "exit") == 0) {
            break;
        }

        parse_command(command, args, &input_file, &output_file, &append_mode);
        execute_command(args, input_file, output_file, append_mode);
    }

    return 0;
}
