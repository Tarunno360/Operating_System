#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
    // Remove newline character if present
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }
}

void parse_command(char *command, char **args) {
    int i = 0;
    args[i] = strtok(command, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

void execute_command(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
        }
        exit(1);
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

    while (1) {
        display_prompt();
        read_command(command);
        
        if (strcmp(command, "exit") == 0) {
            break;
        }

        parse_command(command, args);
        execute_command(args);
    }

    return 0;
}