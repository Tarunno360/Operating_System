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
    }
    else if (child_pid == 0) {
        printf("Child process ID: %d, Parent ID: %d\n", getpid(), getppid());

        for (int i = 0; i < 3; i++) {
            grandchild_pid = fork();

            if (grandchild_pid < 0) {
                perror("Fork failed");
                exit(1);
            }
            else if (grandchild_pid == 0) {
                printf("Grandchild process ID: %d, Parent ID: %d\n", getpid(), getppid());
                exit(0);             }
            else {
                wait(NULL);             }
        }

        exit(0);
    }
    else {
        printf("Parent process ID: %d\n", getpid());
        wait(NULL);
    }

    return 0;
}

