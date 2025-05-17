#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define HISTORY_SIZE 100
#define MAX_NUM_PIPES 16

char *history[HISTORY_SIZE];
int history_count = 0;

void command_history_string(const char *cmd) {
    if (history_count < HISTORY_SIZE) {
        history[history_count++] = strdup(cmd);
    }
}

char **tokenize(char *line) {
    char **tokens = malloc(MAX_ARGS * sizeof(char *));
    int pos = 0;
    int length = strlen(line);
    int i = 0;
    char current_token[MAX_INPUT];
    int token_len = 0;

    while (i < length) {
        if (isspace(line[i]) && token_len == 0) {  //surutei space diye dile jate omit hoy
            i++;
            continue;
        }

        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            i++;  
            while (i < length && line[i] != quote) {
                current_token[token_len++] = line[i++];
            }
            if (i < length) i++;  
            while (i < length && !isspace(line[i])) {
                if (line[i] == '\\' && i + 1 < length) { //check for "" and then add the next char quoted jinish gula ke handle kore 
                    i++; 
                    current_token[token_len++] = line[i++];
                } else {
                    current_token[token_len++] = line[i++];
                }
            }
            
            current_token[token_len] = '\0';
            tokens[pos++] = strdup(current_token);
            token_len = 0;
            continue;
        }

        if (line[i] == '\\' && i + 1 < length) {
            i++; 
            current_token[token_len++] = line[i++]; 
            continue;
        }

        if (isspace(line[i]) && token_len > 0) {
            current_token[token_len] = '\0';
            tokens[pos++] = strdup(current_token);
            token_len = 0;
            i++;
            continue;
        }

        if (token_len > 0) {
            current_token[token_len++] = line[i++];
            continue;
        }

        token_len = 0;
        while (i < length && !isspace(line[i])) {

            if (line[i] == '\\' && i + 1 < length) {
                i++;  
                current_token[token_len++] = line[i++];
            } else if (line[i] == '"' || line[i] == '\'') {
                char quote = line[i++];
                while (i < length && line[i] != quote) {
                    current_token[token_len++] = line[i++];
                }
                if (i < length) i++; 
            } else {
                current_token[token_len++] = line[i++];
            }
        }
        
        if (token_len > 0) {
            current_token[token_len] = '\0';
            tokens[pos++] = strdup(current_token);
            token_len = 0;
        }
    }
    
    tokens[pos] = NULL;
    return tokens;
}

void redirect_function_args(char *command, char **args, char **input_file, char **output_file, int *append_mode) {
    int i = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append_mode = 0;

    char **tokens = tokenize(command);
    for (int j = 0; tokens[j] != NULL; j++) {
        if (strcmp(tokens[j], "<") == 0) {
            *input_file = tokens[++j];
        } else if (strcmp(tokens[j], ">") == 0) {
            *output_file = tokens[++j];
            *append_mode = 0;
        } else if (strcmp(tokens[j], ">>") == 0) {
            *output_file = tokens[++j];
            *append_mode = 1;
        } else {
            args[i++] = tokens[j];
        }
    }
    args[i] = NULL;
    free(tokens);
}

int single_function_exec(char *cmd) {
    char *args[MAX_ARGS];
    char *input_file = NULL, *output_file = NULL;
    int append_mode = 0;
    redirect_function_args(cmd, args, &input_file, &output_file, &append_mode);

    if (args[0] == NULL) return 0;

    if (strcmp(args[0], "cd") == 0) {
        if (args[1]) chdir(args[1]);
        else fprintf(stderr, "cd: missing argument\n");
        return 0;
    }

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting my shell ...\n");
        exit(0);
    }

    if (strcmp(args[0], "history") == 0) {
        for (int i = 0; i < history_count; ++i) {
            printf("%d: %s\n", i + 1, history[i]);
        }
        return 0;
    }

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL); 

        if (input_file) {
            int in_fd = open(input_file, O_RDONLY);
            if (in_fd < 0) {
                perror("Sorry failed to open input file. Check your input again");
                exit(1);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (output_file) {
            int out_fd = open(output_file, O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC), 0644);
            if (out_fd < 0) {
                perror("Sorry failed to open output file. Check your input again");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

void pipeline_function(char **commands, int count) {
    int pipefd[2];
    int in_fd = 0;
    pid_t pids[count];

    for (int i = 0; i < count; ++i) {
        pipe(pipefd);
        if ((pids[i] = fork()) == 0) {
            signal(SIGINT, SIG_DFL);  

            dup2(in_fd, STDIN_FILENO);
            if (i < count - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);

            char *args[MAX_ARGS];
            char *input_file = NULL, *output_file = NULL;
            int append_mode = 0;
            redirect_function_args(commands[i], args, &input_file, &output_file, &append_mode);

            if (input_file) {
                int in = open(input_file, O_RDONLY);
                if (in < 0) { perror("open input"); exit(1); }
                dup2(in, STDIN_FILENO); close(in);
            }

            if (output_file) {
                int out = open(output_file, O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC), 0644);
                if (out < 0) { perror("open output"); exit(1); }
                dup2(out, STDOUT_FILENO); close(out);
            }

            execvp(args[0], args);
            perror("exec failed");
            exit(EXIT_FAILURE);
        }
        waitpid(pids[i], NULL, 0);
        close(pipefd[1]);
        in_fd = pipefd[0];
    }
}

void command_chaining_function(char *line) {
    char *semicolon_cmds[MAX_ARGS];
    int sc_count = 0;
    char *sc_token = strtok(line, ";");
    while (sc_token != NULL && sc_count < MAX_ARGS) {
        semicolon_cmds[sc_count++] = sc_token;
        sc_token = strtok(NULL, ";");
    }

    for (int i = 0; i < sc_count; ++i) {
        char *and_cmds[MAX_ARGS];
        int and_count = 0;
        char *and_token = strtok(semicolon_cmds[i], "&&");
        while (and_token != NULL && and_count < MAX_ARGS) {
            and_cmds[and_count++] = and_token;
            and_token = strtok(NULL, "&&");
        }

        int continue_chain = 1;
        for (int j = 0; j < and_count; ++j) {
            if (!continue_chain) break;
            char *pipe_cmds[MAX_ARGS];
            int pipe_count = 0;
            char *pipe_token = strtok(and_cmds[j], "|");
            while (pipe_token != NULL && pipe_count < MAX_ARGS) {
                pipe_cmds[pipe_count++] = pipe_token;
                pipe_token = strtok(NULL, "|");
            }

            if (pipe_count > 1) pipeline_function(pipe_cmds, pipe_count);
            else continue_chain = (single_function_exec(pipe_cmds[0]) == 0);
        }
    }
}

int main() {
    char input[MAX_INPUT];
    signal(SIGINT, SIG_IGN); 

    while (1) {
        printf("mysh> ");
        fflush(stdout); //promt is printing without completing process 

        if (!fgets(input, MAX_INPUT, stdin)) break;
        if (input[0] == '\n') continue;

        input[strcspn(input, "\n")] = 0;
        command_history_string(input);
        command_chaining_function(strdup(input));
    }
    return 0;
}
