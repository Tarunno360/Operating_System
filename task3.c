#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int process_count = 1;  // Start with the initial parent process

void create_extra_child_if_needed() {
    if (getpid() % 2 != 0) {  // Check if PID is odd
        pid_t new_child = fork();
        if (new_child > 0) {
            process_count++;  // Increase count for the new child
        }
    }
}

int main() {
    pid_t a, b, c;

    // First fork
    a = fork();
    if (a > 0) process_count++;

    // Second fork
    b = fork();
    if (b > 0) process_count++;

    // Third fork
    c = fork();
    if (c > 0) process_count++;

    // Check if the process has an odd PID and create another child if needed
    create_extra_child_if_needed();

    // Ensure only the original parent prints the count
    sleep(1); // Ensure all processes execute before the parent prints
    if (getppid() != 1) {  // Only the original parent prints
        printf("Total processes created: %d\n", process_count);
    }

    return 0;
}

