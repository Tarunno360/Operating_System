#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t child_pid, grandchild_pid;

    child_pid = fork();

    if (child_pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (child_pid == 0) {
 
        grandchild_pid = fork();

        if (grandchild_pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (grandchild_pid == 0) {
            printf("I am grandchild\n");
            exit(0); // Grandchild exits
        } else {
            wait(NULL);
            printf("I am child\n");
            exit(0);
        }
    } else {
        wait(NULL);
        printf("I am parent\n");
    }

    return 0;
}

