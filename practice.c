#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t a, b, c;
    int process_count = 1; // Start with the initial parent process

    printf("Parent process PID: %d\n", getpid());

    a = fork();
    b = fork();
    c = fork();

    if (a == 0) { // First child
        printf("First child PID: %d, Parent PID: %d\n", getpid(), getppid());
        if (getpid() % 2 != 0) {
            pid_t d = fork();
            process_count++; // Increment for the grandchild
            if (d == 0) {
                printf("Grandchild (from first child) PID: %d, Parent PID: %d\n", getpid(), getppid());
                exit(0);
            } else if (d > 0) {
                wait(NULL); // Wait for the grandchild
            } else {
                perror("fork");
                exit(1);
            }
        }
        exit(0);
    } else if (a < 0) {
        perror("fork");
        return 1;
    }

    if (b == 0) { // Second child
        printf("Second child PID: %d, Parent PID: %d\n", getpid(), getppid());
        if (getpid() % 2 != 0) {
            pid_t e = fork();
            process_count++; // Increment for the grandchild
            if (e == 0) {
                printf("Grandchild (from second child) PID: %d, Parent PID: %d\n", getpid(), getppid());
                exit(0);
            } else if (e > 0) {
                wait(NULL); // Wait for the grandchild
            } else {
                perror("fork");
                exit(1);
            }
        }
        exit(0);
    } else if (b < 0) {
        perror("fork");
        return 1;
    }

    if (c == 0) { // Third child
        printf("Third child PID: %d, Parent PID: %d\n", getpid(), getppid());
        if (getpid() % 2 != 0) {
            pid_t f = fork();
            process_count++; // Increment for the grandchild
            if (f == 0) {
                printf("Grandchild (from third child) PID: %d, Parent PID: %d\n", getpid(), getppid());
                exit(0);
            } else if (f > 0) {
                wait(NULL); // Wait for the grandchild
            } else {
                perror("fork");
                exit(1);
            }
        }
        exit(0);
    } else if (c < 0) {
        perror("fork");
        return 1;
    }

    // Parent waits for all its direct children to finish
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("\nTotal number of processes created (including the initial parent): %d\n", process_count);

    return 0;
}