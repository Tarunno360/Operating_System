#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int process_count = 1;  

void odd_pid_child_creation() {
    if (getpid() % 2 != 0) {  
        pid_t new_child = fork();
        if (new_child > 0) {
            process_count++;  
        }
    }
}

int main() {
    pid_t a, b, c;

    a = fork();
    if (a > 0) process_count++;

    b = fork();
    if (b > 0) process_count++;

    c = fork();
    if (c > 0) process_count++;
    odd_pid_child_creation();
    while (wait(NULL)>0);
    if (getppid() != 1) {  
        printf("Total processes created: %d\n", process_count);
    }
    return 0;
}

