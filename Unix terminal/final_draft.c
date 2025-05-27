#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_NUM_ARGS 64
#define MAX_NUM_PIPES 16

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

void parse_args_redir(char *command, char **args, char **input_file, char **output_file, int *append_mode) {
    int i = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append_mode = 0;

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
            char *args[MAX_NUM_ARGS];
            char *input_file = NULL;
            char *output_file = NULL;
            int append_mode = 0;

            if (i > 0) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (i < num_cmds - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            parse_args_redir(commands[i], args, &input_file, &output_file, &append_mode);

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
        } else if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }

        if (prev_fd != -1) close(prev_fd);
        if (i < num_cmds - 1) {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }

    for (i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int main() {
    char command_line[MAX_COMMAND_LENGTH];
    char *commands[MAX_NUM_PIPES];

    while (1) {
        display_prompt();
        read_command(command_line);

        if (strcmp(command_line, "exit") == 0) {
            break;
        }

        int num_cmds = split_by_delim(command_line, commands, "|");
        execute_pipeline(commands, num_cmds);
    }

    return 0;
}